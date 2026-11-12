#ifndef NYROX_UEFI_BOOT_SERVICES_H
#define NYROX_UEFI_BOOT_SERVICES_H

#include <stddef.h>
#include "types.h"
#include "guid.h"

#define EFI_BOOT_SERVICES_DATA 4

#define EFI_ALLOCATE_ANY_PAGES 0
#define EFI_ALLOCATE_MAX_ADDRESS 1
#define EFI_ALLOCATE_ADDRESS 2
#define EFI_LOADER_CODE 1
#define EFI_LOADER_DATA 2

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES)(
    UINT32 AllocateType,
    UINT32 MemoryType,
    UINTN Pages,
    UINT64 *Memory
);

typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)(
    UINTN *MemoryMapSize,
    void *MemoryMap,
    UINTN *MapKey,
    UINTN *DescriptorSize,
    UINT32 *DescriptorVersion
);

typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES)(
    EFI_HANDLE ImageHandle,
    UINTN MapKey
);

typedef EFI_STATUS (EFIAPI *EFI_FREE_PAGES)(
    UINT64 Memory,
    UINTN Pages
);

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(

    UINT32 PoolType,
    UINTN Size,
    void **Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL)(void *Buffer);

typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL)(
    EFI_GUID *Protocol,
    void *Registration,
    void **Interface
);

typedef struct EFI_BOOT_SERVICES EFI_BOOT_SERVICES;

struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;
    void *RaiseTPL;
    void *RestoreTPL;
    EFI_ALLOCATE_PAGES AllocatePages;
    EFI_FREE_PAGES FreePages;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    EFI_FREE_POOL FreePool;
    void *CreateEvent;
    void *SetTimer;
    void *WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;
    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    void *HandleProtocol;
    void *Reserved;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;
    void *LoadImage;
    void *StartImage;
    void *Exit;
    void *UnloadImage;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;
    void *GetNextMonotonicCount;
    void *Stall;
    void *SetWatchdogTimer;
    void *ConnectController;
    void *DisconnectController;
    void *OpenProtocol;
    void *CloseProtocol;
    void *OpenProtocolInformation;
    void *ProtocolsPerHandle;
    void *LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL LocateProtocol;
    void *InstallMultipleProtocolInterfaces;
    void *UninstallMultipleProtocolInterfaces;
    void *CalculateCrc32;
    void *CopyMem;
    void *SetMem;
    void *CreateEventEx;
};

_Static_assert(sizeof(EFI_BOOT_SERVICES) == 376,
               "Boot Services size error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, GetMemoryMap) == 56,
               "GetMemoryMap offset error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, FreePages) == 48,
               "FreePages offset error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, AllocatePages) == 40,
               "AllocatePages offset error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, HandleProtocol) == 152,
               "HandleProtocol offset error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, ExitBootServices) == 232,
               "ExitBootServices offset error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, LocateHandleBuffer) == 312,
               "LocateHandleBuffer offset error");
_Static_assert(offsetof(EFI_BOOT_SERVICES, LocateProtocol) == 320,
               "LocateProtocol offset error");

#endif
