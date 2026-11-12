#include "uefi/uefi.h"
#include "uefi/simple_fs.h"

EFI_STATUS locate_filesystem(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **FileSystem
) {
    if (SystemTable == 0 ||
        SystemTable->BootServices == 0 ||
        FileSystem == 0) {
        return (EFI_STATUS)1;
    }

    EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    *FileSystem = 0;

    return SystemTable->BootServices->LocateProtocol(
        &guid,
        0,
        (void **)FileSystem
    );
}
