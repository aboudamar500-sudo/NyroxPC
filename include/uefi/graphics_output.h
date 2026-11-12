#ifndef NYROX_UEFI_GRAPHICS_OUTPUT_H
#define NYROX_UEFI_GRAPHICS_OUTPUT_H

#include "types.h"
#include "guid.h"

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
    { 0x9042a9de, 0x23dc, 0x4a38, \
      { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } }

#define PIXEL_RED_GREEN_BLUE_RESERVED8 0
#define PIXEL_BLUE_GREEN_RED_RESERVED8 1

typedef struct {
    UINT32 RedMask;
    UINT32 GreenMask;
    UINT32 BlueMask;
    UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
    UINT32 Version;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelFormat;
    EFI_PIXEL_BITMASK PixelInformation;
    UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32 MaxMode;
    UINT32 Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN SizeOfInfo;
    UINT64 FrameBufferBase;
    UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_QUERY_MODE)(
    EFI_GRAPHICS_OUTPUT_PROTOCOL *, UINT32, UINTN *,
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **
);

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_SET_MODE)(
    EFI_GRAPHICS_OUTPUT_PROTOCOL *, UINT32
);

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_BLT)(
    EFI_GRAPHICS_OUTPUT_PROTOCOL *, void *, UINT32,
    UINTN, UINTN, UINTN, UINTN, UINTN, UINTN, UINTN
);

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_QUERY_MODE QueryMode;
    EFI_GRAPHICS_OUTPUT_SET_MODE SetMode;
    EFI_GRAPHICS_OUTPUT_BLT Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
};

_Static_assert(sizeof(EFI_PIXEL_BITMASK) == 16, "GOP pixel mask size");
_Static_assert(sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) == 36,
               "GOP mode information size");
_Static_assert(sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE) == 40,
               "GOP mode size");
_Static_assert(sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL) == 32,
               "GOP protocol size");

#endif
