#ifndef NYROX_UEFI_FILE_INFO_H
#define NYROX_UEFI_FILE_INFO_H

#include "types.h"

#define EFI_FILE_INFO_FIXED_SIZE 80
#define NYROX_MAX_KERNEL_SIZE (64ULL * 1024ULL * 1024ULL)

#define EFI_FILE_INFO_ID \
    { 0x09576e92, 0x6d3f, 0x11d2, \
      { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

typedef struct {
    UINT64 Size;
    UINT64 FileSize;
    UINT64 PhysicalSize;
    UINT64 CreateTime[2];
    UINT64 LastAccessTime[2];
    UINT64 ModificationTime[2];
    UINT64 Attribute;
    CHAR16 FileName[];
} EFI_FILE_INFO;

_Static_assert(__builtin_offsetof(EFI_FILE_INFO, FileName) == 80,
               "EFI_FILE_INFO layout error");

#endif
