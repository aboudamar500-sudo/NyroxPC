#include "uefi/file.h"

EFI_STATUS close_file(EFI_FILE_PROTOCOL *File) {
    if (File == 0)
        return EFI_SUCCESS;

    if (File->Close == 0)
        return (EFI_STATUS)2;

    return File->Close(File);
}
