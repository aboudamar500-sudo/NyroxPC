#include "uefi/uefi.h"
#include "uefi/boot_services.h"
#include "elf64.h"

#define NYROX_PAGE_SIZE 4096ULL

int validate_elf64(const void *buffer, Elf64_U64 file_size);

typedef struct {
    UINT64 base;
    UINTN pages;
} ElfLoadedSegment;

EFI_STATUS load_elf64(
    EFI_SYSTEM_TABLE *SystemTable,
    const void *FileBuffer,
    UINT64 FileSize,
    UINT64 *EntryPoint,
    ElfLoadedSegment *Loaded,
    UINTN *LoadedCount
) {
    if (!SystemTable || !SystemTable->BootServices ||
        !FileBuffer || !EntryPoint || !Loaded || !LoadedCount)
        return EFI_INVALID_PARAMETER;

    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    if (!bs->AllocatePages || !bs->FreePages)
        return EFI_INVALID_PARAMETER;

    *EntryPoint = 0;
    *LoadedCount = 0;

    if (!validate_elf64(FileBuffer, FileSize))
        return EFI_INVALID_PARAMETER;

    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)FileBuffer;
    const Elf64_Phdr *phdrs =
        (const Elf64_Phdr *)((const Elf64_U8 *)FileBuffer +
                             eh->program_header_offset);

    for (UINTN i = 0; i < eh->program_header_count; ++i) {
        const Elf64_Phdr *ph = &phdrs[i];

        if (ph->type != ELF64_PT_LOAD)
            continue;

        if (ph->virtual_address < NYROX_PAGE_SIZE ||
            ph->memory_size == 0 ||
            ph->memory_size > 64ULL * 1024ULL * 1024ULL ||
            ph->file_size > ph->memory_size ||
            ph->virtual_address > ~(UINT64)0 - ph->memory_size)
            goto invalid;

        UINT64 page_base =
            ph->virtual_address & ~(NYROX_PAGE_SIZE - 1);

        UINT64 offset = ph->virtual_address - page_base;

        if (ph->memory_size > ~(UINT64)0 - offset)
            goto invalid;

        UINT64 span = offset + ph->memory_size;

        if (span > ~(UINT64)0 - (NYROX_PAGE_SIZE - 1))
            goto invalid;

        UINT64 pages64 = (span / NYROX_PAGE_SIZE) +
            ((span % NYROX_PAGE_SIZE) != 0);

        if (pages64 == 0 || pages64 > ~(UINTN)0)
            goto invalid;

        if (*LoadedCount >= ELF64_MAX_PROGRAM_HEADERS)
            goto invalid;

        if (pages64 > ~(UINT64)0 / NYROX_PAGE_SIZE)
            goto invalid;

        UINT64 page_bytes = pages64 * NYROX_PAGE_SIZE;
        if (page_bytes > ~(UINT64)0 - page_base)
            goto invalid;

        UINT64 new_end = page_base + page_bytes;

        for (UINTN k = 0; k < *LoadedCount; ++k) {
            if ((UINT64)Loaded[k].pages > ~(UINT64)0 / NYROX_PAGE_SIZE)
                goto invalid;

            UINT64 old_bytes =
                (UINT64)Loaded[k].pages * NYROX_PAGE_SIZE;

            if (old_bytes > ~(UINT64)0 - Loaded[k].base)
                goto invalid;

            UINT64 old_end = Loaded[k].base + old_bytes;

            if (page_base < old_end && Loaded[k].base < new_end)
                goto invalid;
        }

        UINT64 address = page_base;

        EFI_STATUS status = bs->AllocatePages(
            EFI_ALLOCATE_ADDRESS,
            EFI_LOADER_CODE,
            (UINTN)pages64,
            &address
        );

        if (status != EFI_SUCCESS || address != page_base) {
            if (status == EFI_SUCCESS)
                bs->FreePages(address, (UINTN)pages64);
            goto allocation_failed;
        }

        Loaded[*LoadedCount].base = address;
        Loaded[*LoadedCount].pages = (UINTN)pages64;
        (*LoadedCount)++;

        UINT8 *destination = (UINT8 *)(UINTN)(address + offset);
        const UINT8 *source =
            (const UINT8 *)FileBuffer + ph->offset;

        for (UINT64 j = 0; j < ph->file_size; ++j)
            destination[j] = source[j];

        for (UINT64 j = ph->file_size; j < ph->memory_size; ++j)
            destination[j] = 0;
    }

    *EntryPoint = eh->entry;
    return EFI_SUCCESS;

invalid:
    while (*LoadedCount > 0) {
        --(*LoadedCount);
        bs->FreePages(
            Loaded[*LoadedCount].base,
            Loaded[*LoadedCount].pages
        );
    }
    return EFI_INVALID_PARAMETER;

allocation_failed:
    while (*LoadedCount > 0) {
        --(*LoadedCount);
        bs->FreePages(
            Loaded[*LoadedCount].base,
            Loaded[*LoadedCount].pages
        );
    }
    return EFI_INVALID_PARAMETER;
}
