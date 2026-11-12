#include "uefi/uefi.h"
#include "uefi/simple_fs.h"

EFI_STATUS open_root_volume(
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem,
    EFI_FILE_PROTOCOL **Root
) {
    if (FileSystem == 0 ||
        FileSystem->OpenVolume == 0 ||
        Root == 0) {
        return (EFI_STATUS)2;
    }

    *Root = 0;

    EFI_STATUS status = FileSystem->OpenVolume(
        FileSystem,
        Root
    );

    if (status != EFI_SUCCESS)
        return status;

    if (*Root == 0)
        return (EFI_STATUS)2;

    return EFI_SUCCESS;
}
