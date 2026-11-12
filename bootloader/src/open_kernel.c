#include "uefi/file.h"

static CHAR16 kernel_path[] = {
    '\\', 'E', 'F', 'I', '\\',
    'N', 'Y', 'R', 'O', 'X', '\\',
    'K', 'E', 'R', 'N', 'E', 'L', '.', 'E', 'L', 'F', 0
};

EFI_STATUS open_kernel_file(
    EFI_FILE_PROTOCOL *Root,
    EFI_FILE_PROTOCOL **Kernel
) {
    if (Root == 0 || Root->Open == 0 || Kernel == 0)
        return (EFI_STATUS)2;

    *Kernel = 0;

    EFI_STATUS status = Root->Open(
        Root,
        Kernel,
        kernel_path,
        EFI_FILE_MODE_READ,
        0
    );

    if (status != EFI_SUCCESS)
        return status;

    if (*Kernel == 0)
        return (EFI_STATUS)2;

    return EFI_SUCCESS;
}
