#include "memory_manager.h"

#define MM_PAGE_SIZE 4096ULL
#define MM_MAX_ADDRESS (4ULL * 1024ULL * 1024ULL * 1024ULL)
#define MM_MAX_PAGES (MM_MAX_ADDRESS / MM_PAGE_SIZE)
#define MM_BITMAP_BYTES (MM_MAX_PAGES / 8ULL)
#define MM_FIRST_PAGE (0x100000ULL / MM_PAGE_SIZE)

typedef unsigned char u8;

typedef struct {
    u32 type;
    u32 padding;
    u64 physical_start;
    u64 virtual_start;
    u64 number_of_pages;
    u64 attribute;
} MmMemoryDescriptor;

static u8 used_bitmap[MM_BITMAP_BYTES];
static u8 managed_bitmap[MM_BITMAP_BYTES];

static u64 allocation_hint = MM_FIRST_PAGE;
static int initialized = 0;

static int bit_get(const u8 *bitmap, u64 page) {
    return (bitmap[page / 8] >> (page % 8)) & 1;
}

static void bit_set(u8 *bitmap, u64 page) {
    bitmap[page / 8] |= (u8)(1U << (page % 8));
}

static void bit_clear(u8 *bitmap, u64 page) {
    bitmap[page / 8] &= (u8)~(1U << (page % 8));
}

int mm_init(const NyroxBootInfo *info) {
    initialized = 0;

    if (!info || !info->memory_map ||
        !info->memory_map_size ||
        info->memory_map_descriptor_size < sizeof(MmMemoryDescriptor) ||
        info->memory_map_size % info->memory_map_descriptor_size)
        return 0;

    for (u64 i = 0; i < MM_BITMAP_BYTES; ++i) {
        used_bitmap[i] = 0xFF;
        managed_bitmap[i] = 0;
    }

    const u8 *cursor = (const u8 *)info->memory_map;
    u64 offset = 0;
    u64 managed_pages = 0;

    while (offset < info->memory_map_size) {
        const MmMemoryDescriptor *entry =
            (const MmMemoryDescriptor *)(cursor + offset);

        if (entry->type == 7 && entry->number_of_pages) {
            if (entry->number_of_pages > (~0ULL / MM_PAGE_SIZE))
                return 0;

            u64 bytes = entry->number_of_pages * MM_PAGE_SIZE;

            if (entry->physical_start > ~0ULL - bytes)
                return 0;

            u64 end_address = entry->physical_start + bytes;

            u64 first = entry->physical_start / MM_PAGE_SIZE;
            if (entry->physical_start % MM_PAGE_SIZE)
                ++first;

            u64 last = end_address / MM_PAGE_SIZE;

            if (first < MM_FIRST_PAGE)
                first = MM_FIRST_PAGE;

            if (last > MM_MAX_PAGES)
                last = MM_MAX_PAGES;

            for (u64 page = first; page < last; ++page) {
                if (!bit_get(managed_bitmap, page)) {
                    bit_set(managed_bitmap, page);
                    bit_clear(used_bitmap, page);
                    ++managed_pages;
                }
            }
        }

        offset += info->memory_map_descriptor_size;
    }

    if (!managed_pages)
        return 0;

    allocation_hint = MM_FIRST_PAGE;
    initialized = 1;
    return 1;
}

u64 mm_alloc_page(void) {
    if (!initialized)
        return 0;

    for (u64 page = allocation_hint; page < MM_MAX_PAGES; ++page) {
        if (bit_get(managed_bitmap, page) &&
            !bit_get(used_bitmap, page)) {
            bit_set(used_bitmap, page);
            allocation_hint = page + 1;
            return page * MM_PAGE_SIZE;
        }
    }

    for (u64 page = MM_FIRST_PAGE; page < allocation_hint; ++page) {
        if (bit_get(managed_bitmap, page) &&
            !bit_get(used_bitmap, page)) {
            bit_set(used_bitmap, page);
            allocation_hint = page + 1;
            return page * MM_PAGE_SIZE;
        }
    }

    return 0;
}

int mm_free_page(u64 address) {
    if (!initialized ||
        address % MM_PAGE_SIZE ||
        address < 0x100000ULL ||
        address >= MM_MAX_ADDRESS)
        return 0;

    u64 page = address / MM_PAGE_SIZE;

    if (!bit_get(managed_bitmap, page) ||
        !bit_get(used_bitmap, page))
        return 0;

    bit_clear(used_bitmap, page);

    if (page < allocation_hint)
        allocation_hint = page;

    return 1;
}

u64 mm_free_pages_count(void) {
    if (!initialized)
        return 0;

    u64 count = 0;

    for (u64 page = MM_FIRST_PAGE; page < MM_MAX_PAGES; ++page) {
        if (bit_get(managed_bitmap, page) &&
            !bit_get(used_bitmap, page))
            ++count;
    }

    return count;
}
