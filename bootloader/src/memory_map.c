#include "uefi/uefi.h"
#include "uefi/boot_services.h"
#include "uefi/memory.h"

#define NYROX_MEMORY_MAP_INITIAL_SIZE (256ULL * 1024ULL)
#define NYROX_MEMORY_MAP_EXTRA_SIZE   (64ULL * 1024ULL)

typedef struct {
    EFI_MEMORY_DESCRIPTOR *Buffer;
    UINTN Size;
    UINTN Capacity;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} NyroxMemoryMap;

EFI_STATUS refresh_memory_map(
    EFI_SYSTEM_TABLE *SystemTable,
    NyroxMemoryMap *Result
) {
    if (!SystemTable || !SystemTable->BootServices || !Result ||
        !Result->Buffer || !Result->Capacity)
        return EFI_INVALID_PARAMETER;

    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;
    UINTN size = Result->Capacity;
    UINTN key = 0;
    UINTN descriptor_size = 0;
    UINT32 version = 0;

    EFI_STATUS status = bs->GetMemoryMap(
        &size,
        Result->Buffer,
        &key,
        &descriptor_size,
        &version
    );

    if (status != EFI_SUCCESS)
        return status;

    if (descriptor_size < sizeof(EFI_MEMORY_DESCRIPTOR) ||
        version != EFI_MEMORY_DESCRIPTOR_VERSION)
        return EFI_INVALID_PARAMETER;

    Result->Size = size;
    Result->MapKey = key;
    Result->DescriptorSize = descriptor_size;
    Result->DescriptorVersion = version;

    return EFI_SUCCESS;
}

EFI_STATUS get_memory_map(
    EFI_SYSTEM_TABLE *SystemTable,
    NyroxMemoryMap *Result
) {
    if (!SystemTable || !SystemTable->BootServices || !Result)
        return EFI_INVALID_PARAMETER;

    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    if (!bs->AllocatePool || !bs->FreePool || !bs->GetMemoryMap)
        return EFI_INVALID_PARAMETER;

    Result->Buffer = 0;
    Result->Size = 0;
    Result->Capacity = 0;
    Result->MapKey = 0;
    Result->DescriptorSize = 0;
    Result->DescriptorVersion = 0;

    UINTN capacity = NYROX_MEMORY_MAP_INITIAL_SIZE;
    void *buffer = 0;

    EFI_STATUS status = bs->AllocatePool(
        EFI_LOADER_DATA, capacity, &buffer
    );

    if (status != EFI_SUCCESS)
        return status;

    for (UINTN attempt = 0; attempt < 4; ++attempt) {
        UINTN map_size = capacity;
        UINTN map_key = 0;
        UINTN descriptor_size = 0;
        UINT32 descriptor_version = 0;

        status = bs->GetMemoryMap(
            &map_size,
            buffer,
            &map_key,
            &descriptor_size,
            &descriptor_version
        );

        if (status == EFI_SUCCESS) {
            if (descriptor_size < sizeof(EFI_MEMORY_DESCRIPTOR) ||
                descriptor_version != EFI_MEMORY_DESCRIPTOR_VERSION) {
                bs->FreePool(buffer);
                return EFI_INVALID_PARAMETER;
            }

            Result->Buffer = (EFI_MEMORY_DESCRIPTOR *)buffer;
            Result->Size = map_size;
            Result->Capacity = capacity;
            Result->MapKey = map_key;
            Result->DescriptorSize = descriptor_size;
            Result->DescriptorVersion = descriptor_version;
            return EFI_SUCCESS;
        }

        if (status != EFI_BUFFER_TOO_SMALL) {
            bs->FreePool(buffer);
            return status;
        }

        if (bs->FreePool(buffer) != EFI_SUCCESS)
            return EFI_INVALID_PARAMETER;

        if (map_size > ~(UINTN)0 - NYROX_MEMORY_MAP_EXTRA_SIZE)
            return EFI_INVALID_PARAMETER;

        capacity = map_size + NYROX_MEMORY_MAP_EXTRA_SIZE;

        status = bs->AllocatePool(
            EFI_LOADER_DATA, capacity, &buffer
        );

        if (status != EFI_SUCCESS)
            return status;
    }

    bs->FreePool(buffer);
    return EFI_BUFFER_TOO_SMALL;
}
