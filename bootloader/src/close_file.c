#include "uefi/file.h"

EFI_STATUS close_file(EFI_FILE_PROTOCOL *File) {
    if (File == 0)
        return EFI_SUCCESS;

    if (File->Close == 0)
        return EFI_INVALID_PARAMETER;

    return File->Close(File);
}
