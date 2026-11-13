#ifndef NYROX_UEFI_LOADED_IMAGE_H
#define NYROX_UEFI_LOADED_IMAGE_H

#include <stddef.h>
#include "types.h"
#include "guid.h"

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    { 0x5b1b31a1, 0x9562, 0x11d2, \
      { 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

typedef struct {
    UINT32 Revision;
    EFI_HANDLE ParentHandle;
    void *SystemTable;
    EFI_HANDLE DeviceHandle;
    void *FilePath;
    void *Reserved;
    UINT32 LoadOptionsSize;
    void *LoadOptions;
    void *ImageBase;
    UINT64 ImageSize;
    UINT32 ImageCodeType;
    UINT32 ImageDataType;
    void *Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

_Static_assert(offsetof(EFI_LOADED_IMAGE_PROTOCOL, DeviceHandle) == 24,
               "Loaded Image DeviceHandle offset error");

#endif
