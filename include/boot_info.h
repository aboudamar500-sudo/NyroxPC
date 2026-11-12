#ifndef NYROX_BOOT_INFO_H
#define NYROX_BOOT_INFO_H

typedef unsigned long long u64;
typedef unsigned int u32;

#define NYROX_BOOT_INFO_VERSION 1
#define NYROX_PIXEL_FORMAT_RGB 0
#define NYROX_PIXEL_FORMAT_BGR 1

typedef struct {
    u32 structure_version;
    u64 framebuffer_base;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u32 framebuffer_pixels_per_scanline;
    u32 framebuffer_pixel_format;
    u64 memory_map;
    u64 memory_map_size;
    u64 memory_map_descriptor_size;
    u32 memory_map_descriptor_version;
} NyroxBootInfo;

#endif
