#ifndef NYROX_IDT_H
#define NYROX_IDT_H

#include "boot_info.h"

typedef unsigned char u8;
typedef unsigned short u16;

#define IDT_ENTRIES 256
#define IDT_KERNEL_CODE_SELECTOR 0x08
#define IDT_INTERRUPT_GATE 0x8E

#define EXCEPTION_COUNT 32

typedef struct __attribute__((packed)) {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 attributes;
    u16 offset_middle;
    u32 offset_high;
    u32 reserved;
} IdtEntry;

typedef struct __attribute__((packed)) {
    u16 limit;
    u64 base;
} IdtPointer;

void idt_init(void);
typedef struct {
    u64 r15, r14, r13, r12;
    u64 r11, r10, r9, r8;
    u64 rdi, rsi, rbp, rdx;
    u64 rcx, rbx, rax;
    u64 vector;
    u64 error_code;
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} ExceptionFrame;

void exception_handler(const ExceptionFrame *frame);

#endif
