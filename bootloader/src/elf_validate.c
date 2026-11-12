#include "elf64.h"

int validate_elf64(const void *buffer, Elf64_U64 file_size) {
    if (buffer == 0 || file_size < sizeof(Elf64_Ehdr))
        return 0;

    const Elf64_Ehdr *header = (const Elf64_Ehdr *)buffer;

    if (header->ident[0] != 0x7F ||
        header->ident[1] != 'E' ||
        header->ident[2] != 'L' ||
        header->ident[3] != 'F' ||
        header->ident[4] != ELF64_CLASS_64 ||
        header->ident[5] != ELF64_LITTLE_ENDIAN ||
        header->ident[6] != ELF64_VERSION_CURRENT)
        return 0;

    if (header->type != ELF64_TYPE_EXEC ||
        header->machine != ELF64_MACHINE_X86_64 ||
        header->version != ELF64_VERSION_CURRENT)
        return 0;

    if (header->header_size != sizeof(Elf64_Ehdr) ||
        header->program_header_size != sizeof(Elf64_Phdr) ||
        header->program_header_count == 0 ||
        header->program_header_count > ELF64_MAX_PROGRAM_HEADERS)
        return 0;

    if (header->program_header_offset > file_size)
        return 0;

    Elf64_U64 table_size =
        (Elf64_U64)header->program_header_count *
        sizeof(Elf64_Phdr);

    if (table_size > file_size - header->program_header_offset)
        return 0;

    const Elf64_Phdr *phdrs =
        (const Elf64_Phdr *)(
            (const Elf64_U8 *)buffer +
            header->program_header_offset
        );

    int load_segments = 0;
    int entry_is_executable = 0;

    for (Elf64_U16 i = 0;
         i < header->program_header_count;
         i++) {

        const Elf64_Phdr *ph = &phdrs[i];

        if (ph->type != ELF64_PT_LOAD)
            continue;

        load_segments++;

        if (ph->file_size > ph->memory_size ||
            ph->memory_size == 0)
            return 0;

        if (ph->offset > file_size ||
            ph->file_size > file_size - ph->offset)
            return 0;

        if (ph->virtual_address >
            ~(Elf64_U64)0 - ph->memory_size)
            return 0;

        if (ph->alignment > 1) {
            if ((ph->alignment & (ph->alignment - 1)) != 0)
                return 0;

            if (ph->virtual_address % ph->alignment !=
                ph->offset % ph->alignment)
                return 0;
        }

        if ((ph->flags & ELF64_PF_EXECUTE) != 0 &&
            header->entry >= ph->virtual_address &&
            header->entry - ph->virtual_address < ph->memory_size) {
            entry_is_executable = 1;
        }
    }

    return load_segments > 0 && entry_is_executable;
}
