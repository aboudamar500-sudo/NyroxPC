#include "uefi/uefi.h"
#include "uefi/file.h"
#include "uefi/file_info.h"

EFI_STATUS get_file_info(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_FILE_PROTOCOL *File,
    EFI_FILE_INFO **Info
) {
    if (SystemTable == 0 ||
        SystemTable->BootServices == 0 ||
        File == 0 || File->GetInfo == 0 || Info == 0) {
        return EFI_INVALID_PARAMETER;
    }

    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    if (bs->AllocatePool == 0 || bs->FreePool == 0)
        return EFI_INVALID_PARAMETER;

    EFI_GUID guid = EFI_FILE_INFO_ID;
    UINTN size = 0;

    *Info = 0;

    EFI_STATUS status = File->GetInfo(File, &guid, &size, 0);

    if (status != EFI_BUFFER_TOO_SMALL ||
        size < EFI_FILE_INFO_FIXED_SIZE) {
        return status;
    }

    if (size > 4096)
        return EFI_INVALID_PARAMETER;

    UINTN allocated_size = size;
    EFI_FILE_INFO *info = 0;

    status = bs->AllocatePool(
        EFI_BOOT_SERVICES_DATA,
        allocated_size,
        (void **)&info
    );

    if (status != EFI_SUCCESS)
        return status;

    size = allocated_size;

    status = File->GetInfo(File, &guid, &size, info);

    if (status != EFI_SUCCESS) {
        bs->FreePool(info);
        return status;
    }

    if (size > allocated_size ||
        info->Size < EFI_FILE_INFO_FIXED_SIZE ||
        info->Size > allocated_size ||
        info->FileSize == 0 ||
        info->FileSize > NYROX_MAX_KERNEL_SIZE) {
        bs->FreePool(info);
        return EFI_INVALID_PARAMETER;
    }

    *Info = info;
    return EFI_SUCCESS;
}
