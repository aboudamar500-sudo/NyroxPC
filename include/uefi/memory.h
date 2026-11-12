#ifndef NYROX_UEFI_MEMORY_H
#define NYROX_UEFI_MEMORY_H

#include "types.h"

typedef struct {
    UINT32 Type;
    UINT32 Padding;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

_Static_assert(sizeof(EFI_MEMORY_DESCRIPTOR) == 40,
               "EFI_MEMORY_DESCRIPTOR size error");

#endif
