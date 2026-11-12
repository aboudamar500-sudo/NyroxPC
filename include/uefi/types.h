#ifndef NYROX_UEFI_TYPES_H
#define NYROX_UEFI_TYPES_H

typedef unsigned char BOOLEAN;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned long long UINTN;
typedef unsigned long long EFI_STATUS;
typedef void *EFI_HANDLE;
typedef UINT16 CHAR16;

typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0
#define EFI_INVALID_PARAMETER 2
#define EFI_BUFFER_TOO_SMALL 5
#define EFI_MEMORY_DESCRIPTOR_VERSION 1

#endif
