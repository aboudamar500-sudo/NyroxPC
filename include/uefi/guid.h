#ifndef NYROX_UEFI_GUID_H
#define NYROX_UEFI_GUID_H

#include "types.h"

typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8 Data4[8];
} EFI_GUID;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    { 0x964e5b22, 0x6459, 0x11d2, \
      { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

_Static_assert(sizeof(EFI_GUID) == 16, "EFI_GUID size error");

#endif
