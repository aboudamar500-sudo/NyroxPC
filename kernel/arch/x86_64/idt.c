#include "idt.h"

_Static_assert(sizeof(IdtEntry) == 16, "IDT entry must be 16 bytes");
_Static_assert(sizeof(IdtPointer) == 10, "IDTR must be 10 bytes");

typedef void (*IsrHandler)(void);

#define DECLARE_ISR(n) extern void nyrox_isr_##n(void)

DECLARE_ISR(0);
DECLARE_ISR(1);
DECLARE_ISR(2);
DECLARE_ISR(3);
DECLARE_ISR(4);
DECLARE_ISR(5);
DECLARE_ISR(6);
DECLARE_ISR(7);
DECLARE_ISR(8);
DECLARE_ISR(9);
DECLARE_ISR(10);
DECLARE_ISR(11);
DECLARE_ISR(12);
DECLARE_ISR(13);
DECLARE_ISR(14);
DECLARE_ISR(15);
DECLARE_ISR(16);
DECLARE_ISR(17);
DECLARE_ISR(18);
DECLARE_ISR(19);
DECLARE_ISR(20);
DECLARE_ISR(21);
DECLARE_ISR(22);
DECLARE_ISR(23);
DECLARE_ISR(24);
DECLARE_ISR(25);
DECLARE_ISR(26);
DECLARE_ISR(27);
DECLARE_ISR(28);
DECLARE_ISR(29);
DECLARE_ISR(30);
DECLARE_ISR(31);

extern void nyrox_default_stub(void);

static IsrHandler exception_handlers[EXCEPTION_COUNT] = {
    nyrox_isr_0, nyrox_isr_1, nyrox_isr_2, nyrox_isr_3,
    nyrox_isr_4, nyrox_isr_5, nyrox_isr_6, nyrox_isr_7,
    nyrox_isr_8, nyrox_isr_9, nyrox_isr_10, nyrox_isr_11,
    nyrox_isr_12, nyrox_isr_13, nyrox_isr_14, nyrox_isr_15,
    nyrox_isr_16, nyrox_isr_17, nyrox_isr_18, nyrox_isr_19,
    nyrox_isr_20, nyrox_isr_21, nyrox_isr_22, nyrox_isr_23,
    nyrox_isr_24, nyrox_isr_25, nyrox_isr_26, nyrox_isr_27,
    nyrox_isr_28, nyrox_isr_29, nyrox_isr_30, nyrox_isr_31
};

static IdtEntry idt[IDT_ENTRIES] __attribute__((aligned(16)));

void idt_init(void) {
    for (u32 i = 0; i < IDT_ENTRIES; ++i) {
        u64 address = i < EXCEPTION_COUNT
            ? (u64)exception_handlers[i]
            : (u64)nyrox_default_stub;

        idt[i].offset_low = (u16)address;
        idt[i].selector = IDT_KERNEL_CODE_SELECTOR;
        idt[i].ist = (i == 8) ? 1 : 0;
        idt[i].attributes = IDT_INTERRUPT_GATE;
        idt[i].offset_middle = (u16)(address >> 16);
        idt[i].offset_high = (u32)(address >> 32);
        idt[i].reserved = 0;
    }

    IdtPointer pointer = {
        .limit = sizeof(idt) - 1,
        .base = (u64)idt
    };

    __asm__ volatile ("lidt %0" : : "m"(pointer) : "memory");
}
