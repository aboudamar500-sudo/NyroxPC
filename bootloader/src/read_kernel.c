#include "uefi/uefi.h"
#include "uefi/file.h"
#include "uefi/file_info.h"

EFI_STATUS read_kernel_file(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_FILE_PROTOCOL *Kernel,
    UINT64 FileSize,
    void **KernelBuffer
) {
    if (SystemTable == 0 ||
        SystemTable->BootServices == 0 ||
        Kernel == 0 || Kernel->Read == 0 ||
        KernelBuffer == 0 ||
        FileSize == 0 ||
        FileSize > NYROX_MAX_KERNEL_SIZE) {
        return (EFI_STATUS)2;
    }

    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    if (bs->AllocatePool == 0 || bs->FreePool == 0)
        return (EFI_STATUS)2;

    *KernelBuffer = 0;

    void *buffer = 0;

    EFI_STATUS status = bs->AllocatePool(
        EFI_BOOT_SERVICES_DATA,
        (UINTN)FileSize,
        &buffer
    );

    if (status != EFI_SUCCESS)
        return status;

    if (buffer == 0)
        return (EFI_STATUS)2;

    UINTN total_read = 0;

    while (total_read < (UINTN)FileSize) {
        UINTN remaining = (UINTN)FileSize - total_read;

        status = Kernel->Read(
            Kernel,
            &remaining,
            (UINT8 *)buffer + total_read
        );

        if (status != EFI_SUCCESS || remaining == 0) {
            bs->FreePool(buffer);
            return status != EFI_SUCCESS ? status : (EFI_STATUS)2;
        }

        if (remaining > (UINTN)FileSize - total_read) {
            bs->FreePool(buffer);
            return (EFI_STATUS)2;
        }

        total_read += remaining;
    }

    *KernelBuffer = buffer;
    return EFI_SUCCESS;
}
