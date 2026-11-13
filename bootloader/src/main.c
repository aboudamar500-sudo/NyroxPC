#include "uefi/uefi.h"
#include "uefi/loaded_image.h"
#include "uefi/graphics_output.h"
#include "uefi/simple_fs.h"
#include "uefi/file_info.h"
#include "uefi/memory.h"
#include "boot_info.h"
#include "elf64.h"

EFI_STATUS open_root_volume(
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *,
    EFI_FILE_PROTOCOL **
);
EFI_STATUS open_kernel_file(
    EFI_FILE_PROTOCOL *,
    EFI_FILE_PROTOCOL **
);
EFI_STATUS get_file_info(
    EFI_SYSTEM_TABLE *,
    EFI_FILE_PROTOCOL *,
    EFI_FILE_INFO **
);
EFI_STATUS read_kernel_file(
    EFI_SYSTEM_TABLE *,
    EFI_FILE_PROTOCOL *,
    UINT64,
    void **
);
EFI_STATUS close_file(EFI_FILE_PROTOCOL *);

typedef struct {
    UINT64 base;
    UINTN pages;
} ElfLoadedSegment;

typedef struct {
    EFI_MEMORY_DESCRIPTOR *Buffer;
    UINTN Size;
    UINTN Capacity;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} NyroxMemoryMap;

EFI_STATUS load_elf64(
    EFI_SYSTEM_TABLE *,
    const void *,
    UINT64,
    UINT64 *,
    ElfLoadedSegment *,
    UINTN *
);

EFI_STATUS get_memory_map(
    EFI_SYSTEM_TABLE *,
    NyroxMemoryMap *
);

EFI_STATUS refresh_memory_map(
    EFI_SYSTEM_TABLE *,
    NyroxMemoryMap *
);

typedef void (EFIAPI *NYROX_KERNEL_ENTRY)(NyroxBootInfo *);

static void initialize_framebuffer(
    EFI_BOOT_SERVICES *bs,
    NyroxBootInfo *boot_info
) {
    EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;

    EFI_STATUS status = bs->LocateProtocol(
        &guid, 0, (void **)&gop
    );

    if (status != EFI_SUCCESS || !gop || !gop->Mode ||
        !gop->Mode->Info)
        return;

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = gop->Mode->Info;

    if (info->PixelFormat != PIXEL_RED_GREEN_BLUE_RESERVED8 &&
        info->PixelFormat != PIXEL_BLUE_GREEN_RED_RESERVED8)
        return;

    if (!gop->Mode->FrameBufferBase ||
        !info->HorizontalResolution ||
        !info->VerticalResolution ||
        info->PixelsPerScanLine < info->HorizontalResolution)
        return;

    boot_info->framebuffer_base = gop->Mode->FrameBufferBase;
    boot_info->framebuffer_width = info->HorizontalResolution;
    boot_info->framebuffer_height = info->VerticalResolution;
    boot_info->framebuffer_pixels_per_scanline =
        info->PixelsPerScanLine;
    boot_info->framebuffer_pixel_format = info->PixelFormat;
}

static void cleanup(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_FILE_PROTOCOL *Kernel,
    EFI_FILE_PROTOCOL *Root,
    EFI_FILE_INFO *Info,
    void *KernelBuffer,
    NyroxMemoryMap *Map,
    ElfLoadedSegment *Loaded,
    UINTN LoadedCount
) {
    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    if (KernelBuffer)
        bs->FreePool(KernelBuffer);

    if (Info)
        bs->FreePool(Info);

    if (Kernel)
        close_file(Kernel);

    if (Root)
        close_file(Root);

    if (Map && Map->Buffer)
        bs->FreePool(Map->Buffer);

    for (UINTN i = 0; i < LoadedCount; ++i)
        bs->FreePages(Loaded[i].base, Loaded[i].pages);
}


static void uefi_print(EFI_SYSTEM_TABLE *st, CHAR16 *message) {
    if (st && st->ConOut && st->ConOut->OutputString)
        st->ConOut->OutputString(st->ConOut, message);
}

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable
) {
    if (!SystemTable || !SystemTable->BootServices)
        return EFI_INVALID_PARAMETER;

    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = 0;
    EFI_FILE_PROTOCOL *root = 0;
    EFI_FILE_PROTOCOL *kernel = 0;
    EFI_FILE_INFO *info = 0;
    void *kernel_buffer = 0;

    UINT64 kernel_entry = 0;

    ElfLoadedSegment loaded[ELF64_MAX_PROGRAM_HEADERS];
    UINTN loaded_count = 0;

    NyroxMemoryMap map;
    map.Buffer = 0;
    map.Size = 0;
    map.Capacity = 0;
    map.MapKey = 0;
    map.DescriptorSize = 0;
    map.DescriptorVersion = 0;

    NyroxBootInfo boot_info;
    boot_info.structure_version = 0;
    boot_info.framebuffer_base = 0;
    boot_info.framebuffer_width = 0;
    boot_info.framebuffer_height = 0;
    boot_info.framebuffer_pixels_per_scanline = 0;
    boot_info.framebuffer_pixel_format = 0;
    boot_info.memory_map = 0;
    boot_info.memory_map_size = 0;
    boot_info.memory_map_descriptor_size = 0;
    boot_info.memory_map_descriptor_version = 0;

    EFI_STATUS status;
    CHAR16 *failed_stage = L"START\r\n";

    EFI_LOADED_IMAGE_PROTOCOL *loaded_image = 0;
    EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    failed_stage = L"LOADED_IMAGE\r\n";
    status = bs->HandleProtocol(
        ImageHandle,
        &loaded_image_guid,
        (void **)&loaded_image
    );
    if (status != EFI_SUCCESS)
        goto fail;

    if (!loaded_image || !loaded_image->DeviceHandle) {
        status = EFI_INVALID_PARAMETER;
        goto fail;
    }

    failed_stage = L"DEVICE_FILESYSTEM\r\n";
    status = bs->HandleProtocol(
        loaded_image->DeviceHandle,
        &fs_guid,
        (void **)&fs
    );
    if (status != EFI_SUCCESS)
        goto fail;

    failed_stage = L"OPEN_ROOT\r\n";
    status = open_root_volume(fs, &root);
    if (status != EFI_SUCCESS)
        goto fail;

    failed_stage = L"OPEN_KERNEL\r\n";
    status = open_kernel_file(root, &kernel);
    if (status != EFI_SUCCESS)
        goto fail;

    failed_stage = L"FILE_INFO\r\n";
    status = get_file_info(SystemTable, kernel, &info);
    if (status != EFI_SUCCESS)
        goto fail;

    failed_stage = L"READ_KERNEL\r\n";
    status = read_kernel_file(
        SystemTable, kernel, info->FileSize, &kernel_buffer
    );
    if (status != EFI_SUCCESS)
        goto fail;

    failed_stage = L"LOAD_ELF\r\n";
    status = load_elf64(
        SystemTable,
        kernel_buffer,
        info->FileSize,
        &kernel_entry,
        loaded,
        &loaded_count
    );
    if (status != EFI_SUCCESS)
        goto fail;

    /* Release file-related resources before obtaining the final map. */
    bs->FreePool(kernel_buffer);
    kernel_buffer = 0;

    bs->FreePool(info);
    info = 0;

    close_file(kernel);
    kernel = 0;

    close_file(root);
    root = 0;

    boot_info.structure_version = NYROX_BOOT_INFO_VERSION;
    initialize_framebuffer(bs, &boot_info);

    failed_stage = L"GET_MEMORY_MAP\r\n";
    status = get_memory_map(SystemTable, &map);
    if (status != EFI_SUCCESS)
        goto fail;

    boot_info.memory_map = (UINT64)(UINTN)map.Buffer;
    boot_info.memory_map_size = map.Size;
    boot_info.memory_map_descriptor_size = map.DescriptorSize;
    boot_info.memory_map_descriptor_version = map.DescriptorVersion;

    failed_stage = L"EXIT_BOOT_SERVICES\r\n";
    status = bs->ExitBootServices(ImageHandle, map.MapKey);

    if (status == EFI_INVALID_PARAMETER) {
        status = refresh_memory_map(SystemTable, &map);

        if (status != EFI_SUCCESS)
            goto fail;

        boot_info.memory_map_size = map.Size;
        boot_info.memory_map_descriptor_size = map.DescriptorSize;
        boot_info.memory_map_descriptor_version = map.DescriptorVersion;

        status = bs->ExitBootServices(ImageHandle, map.MapKey);
    }

    if (status != EFI_SUCCESS)
        goto fail;

    /* Boot Services are no longer available beyond this point. */
    ((NYROX_KERNEL_ENTRY)(UINTN)kernel_entry)(&boot_info);

    for (;;)
        __asm__ volatile ("cli; hlt");

fail:
    uefi_print(SystemTable, L"NYROX BOOT ERROR: ");
    uefi_print(SystemTable, failed_stage);
    cleanup(
        SystemTable,
        kernel,
        root,
        info,
        kernel_buffer,
        &map,
        loaded,
        loaded_count
    );

    return status;
}
