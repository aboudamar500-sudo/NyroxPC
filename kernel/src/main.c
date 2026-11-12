#include "boot_info.h"

typedef unsigned char u8;

static u8 glyph_row(char c, u32 row) {
    static const u8 font[11][7] = {
        {17,25,21,19,17,17,17}, /* N */
        {17,17,10,4,4,4,4},     /* Y */
        {30,17,17,30,20,18,17}, /* R */
        {14,17,17,17,17,17,14}, /* O */
        {17,17,10,4,10,17,17},  /* X */
        {30,17,17,30,16,16,16}, /* P */
        {14,17,16,16,16,17,14}, /* C */
        {30,17,17,30,17,17,30}, /* B */
        {31,4,4,4,4,4,4},       /* T */
        {31,16,16,30,16,16,31}, /* E */
        {17,17,17,17,17,17,14}  /* D */
    };

    int index = -1;

    switch (c) {
        case 'N': index = 0; break;
        case 'Y': index = 1; break;
        case 'R': index = 2; break;
        case 'O': index = 3; break;
        case 'X': index = 4; break;
        case 'P': index = 5; break;
        case 'C': index = 6; break;
        case 'B': index = 7; break;
        case 'T': index = 8; break;
        case 'E': index = 9; break;
        case 'D': index = 10; break;
        default: return 0;
    }

    return font[index][row];
}

static void put_pixel(
    volatile u8 *fb,
    u32 stride,
    u32 x,
    u32 y,
    u32 format,
    u8 r,
    u8 g,
    u8 b
) {
    volatile u8 *p = fb + ((u64)y * stride + x) * 4;

    if (format == NYROX_PIXEL_FORMAT_RGB) {
        p[0] = r; p[1] = g; p[2] = b;
    } else {
        p[0] = b; p[1] = g; p[2] = r;
    }

    p[3] = 0;
}

static void draw_message(NyroxBootInfo *info) {
    static const char message[] = "NYROX PC BOOTED";

    if (!info->framebuffer_base ||
        !info->framebuffer_width ||
        !info->framebuffer_height ||
        info->framebuffer_pixels_per_scanline < info->framebuffer_width ||
        info->framebuffer_pixel_format > NYROX_PIXEL_FORMAT_BGR)
        return;

    volatile u8 *fb = (volatile u8 *)info->framebuffer_base;

    u32 scale = info->framebuffer_width / 100;
    u32 height_scale = info->framebuffer_height / 12;

    if (height_scale < scale)
        scale = height_scale;
    if (scale < 1)
        scale = 1;
    if (scale > 8)
        scale = 8;

    u32 text_width = 89 * scale;
    u32 text_height = 7 * scale;

    u32 start_x = info->framebuffer_width > text_width
        ? (info->framebuffer_width - text_width) / 2 : 0;
    u32 start_y = info->framebuffer_height > text_height
        ? (info->framebuffer_height - text_height) / 2 : 0;

    for (u32 y = 0; y < info->framebuffer_height; ++y) {
        for (u32 x = 0; x < info->framebuffer_width; ++x) {
            put_pixel(fb, info->framebuffer_pixels_per_scanline,
                      x, y, info->framebuffer_pixel_format,
                      12, 18, 32);
        }
    }

    for (u32 i = 0; message[i]; ++i) {
        for (u32 row = 0; row < 7; ++row) {
            u8 bits = glyph_row(message[i], row);

            for (u32 col = 0; col < 5; ++col) {
                if (!(bits & (1u << (4 - col))))
                    continue;

                for (u32 sy = 0; sy < scale; ++sy) {
                    for (u32 sx = 0; sx < scale; ++sx) {
                        u32 x = start_x + (i * 6 + col) * scale + sx;
                        u32 y = start_y + row * scale + sy;

                        if (x < info->framebuffer_width &&
                            y < info->framebuffer_height) {
                            put_pixel(
                                fb,
                                info->framebuffer_pixels_per_scanline,
                                x, y, info->framebuffer_pixel_format,
                                80, 220, 170
                            );
                        }
                    }
                }
            }
        }
    }
}

void __attribute__((ms_abi))
kernel_main(NyroxBootInfo *boot_info) {
    if (!boot_info ||
        boot_info->structure_version != NYROX_BOOT_INFO_VERSION) {
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }

    draw_message(boot_info);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
