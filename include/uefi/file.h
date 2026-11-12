#ifndef NYROX_UEFI_FILE_H
#define NYROX_UEFI_FILE_H

#include "types.h"
#include "guid.h"

typedef struct EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN)(
    EFI_FILE_PROTOCOL *This,
    EFI_FILE_PROTOCOL **NewHandle,
    CHAR16 *FileName,
    UINT64 OpenMode,
    UINT64 Attributes
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE)(
    EFI_FILE_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO)(
    EFI_FILE_PROTOCOL *This,
    EFI_GUID *InformationType,
    UINTN *BufferSize,
    void *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_READ)(
    EFI_FILE_PROTOCOL *This,
    UINTN *BufferSize,
    void *Buffer
);

struct EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_FILE_OPEN Open;
    EFI_FILE_CLOSE Close;
    void *Delete;
    EFI_FILE_READ Read;
    void *Write;
    void *GetPosition;
    void *SetPosition;
    EFI_FILE_GET_INFO GetInfo;
    void *SetInfo;
    void *Flush;
    void *OpenEx;
    void *ReadEx;
    void *WriteEx;
    void *FlushEx;
};

#define EFI_FILE_MODE_READ 0x0000000000000001ULL

_Static_assert(sizeof(EFI_FILE_PROTOCOL) == 120,
               "EFI_FILE_PROTOCOL size error");
_Static_assert(__builtin_offsetof(EFI_FILE_PROTOCOL, Open) == 8,
               "Open offset error");
_Static_assert(__builtin_offsetof(EFI_FILE_PROTOCOL, Close) == 16,
               "Close offset error");
_Static_assert(__builtin_offsetof(EFI_FILE_PROTOCOL, Read) == 32,
               "Read offset error");
_Static_assert(__builtin_offsetof(EFI_FILE_PROTOCOL, GetInfo) == 64,
               "GetInfo offset error");

#endif
