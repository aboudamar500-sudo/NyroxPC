#include "gdt.h"
#include "boot_info.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct __attribute__((packed)) {
    u64 value;
} GdtEntry;

typedef struct __attribute__((packed)) {
    u16 limit;
    u64 base;
} GdtPointer;

typedef struct __attribute__((packed)) {
    u32 reserved0;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved2;
    u16 reserved3;
    u16 iomap_base;
} Tss;

_Static_assert(sizeof(GdtEntry) == 8, "GDT entry size");
_Static_assert(sizeof(GdtPointer) == 10, "GDTR size");
_Static_assert(sizeof(Tss) == 104, "TSS size");

static GdtEntry gdt[5] __attribute__((aligned(16)));
static Tss tss __attribute__((aligned(16)));
static u8 ist1_stack[16384] __attribute__((aligned(16)));

extern void gdt_load(const GdtPointer *pointer);

u16 gdt_read_tr(void) {
    u16 selector;
    __asm__ volatile ("str %0" : "=r"(selector));
    return selector;
}

void gdt_init(void) {
    gdt[0].value = 0;
    gdt[1].value = 0x00AF9A000000FFFFULL;
    gdt[2].value = 0x00CF92000000FFFFULL;

    tss.rsp0 = 0;
    tss.ist1 = (u64)(ist1_stack + sizeof(ist1_stack));
    tss.iomap_base = sizeof(Tss);

    u64 base = (u64)&tss;
    u64 limit = sizeof(Tss) - 1;

    gdt[3].value =
        (limit & 0xFFFFULL) |
        ((base & 0xFFFFFFULL) << 16) |
        (0x89ULL << 40) |
        (((limit >> 16) & 0x0FULL) << 48) |
        (((base >> 24) & 0xFFULL) << 56);

    gdt[4].value = base >> 32;

    GdtPointer pointer = {
        .limit = sizeof(gdt) - 1,
        .base = (u64)gdt
    };

    gdt_load(&pointer);
}
