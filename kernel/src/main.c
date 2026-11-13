#include "boot_info.h"
#include "memory_manager.h"
#include "idt.h"
#include "gdt.h"
#include "console.h"

typedef unsigned char u8;
typedef unsigned short u16;



static inline void outb(u16 port, u8 value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_init(void) {
    outb(0x3F9, 0x00);
    outb(0x3FB, 0x80);
    outb(0x3F8, 0x01);
    outb(0x3F9, 0x00);
    outb(0x3FB, 0x03);
    outb(0x3FA, 0xC7);
    outb(0x3FC, 0x0B);
}

void serial_write(const char *text) {
    while (*text) {
        while (!(inb(0x3FD) & 0x20)) {}
        outb(0x3F8, (u8)*text++);
    }
}


typedef struct {
    u32 type;
    u32 padding;
    u64 physical_start;
    u64 virtual_start;
    u64 number_of_pages;
    u64 attribute;
} NyroxMemoryDescriptor;

void serial_write_hex(u64 value) {
    static const char digits[] = "0123456789ABCDEF";
    char buffer[16];

    for (u32 i = 0; i < 16; ++i) {
        buffer[15 - i] = digits[value & 0xF];
        value >>= 4;
    }

    for (u32 i = 0; i < 16; ++i) {
        while (!(inb(0x3FD) & 0x20)) {}
        outb(0x3F8, (u8)buffer[i]);
    }
}


static const char *exception_name(u64 vector) {
    switch (vector) {
        case 0: return "Divide Error";
        case 1: return "Debug";
        case 2: return "Non-Maskable Interrupt";
        case 3: return "Breakpoint";
        case 4: return "Overflow";
        case 5: return "Bound Range";
        case 6: return "Invalid Opcode";
        case 7: return "Device Not Available";
        case 8: return "Double Fault";
        case 9: return "Coprocessor Segment Overrun";
        case 10: return "Invalid TSS";
        case 11: return "Segment Not Present";
        case 12: return "Stack-Segment Fault";
        case 13: return "General Protection Fault";
        case 14: return "Page Fault";
        case 16: return "x87 Floating-Point";
        case 17: return "Alignment Check";
        case 18: return "Machine Check";
        case 19: return "SIMD Floating-Point";
        case 20: return "Virtualization";
        case 21: return "Control Protection";
        case 28: return "Hypervisor Injection";
        case 29: return "VMM Communication";
        case 30: return "Security Exception";
        default: return "Reserved or Unknown";
    }
}

void exception_handler(const ExceptionFrame *frame) {
    serial_write("\r\nNYROX: CPU EXCEPTION\r\n");

    serial_write("NYROX: NAME=");
    serial_write(exception_name(frame->vector));
    serial_write("\r\nNYROX: VECTOR=");
    serial_write_hex(frame->vector);

    serial_write("\r\nNYROX: ERROR CODE=");
    serial_write_hex(frame->error_code);

    serial_write("\r\nNYROX: RIP=");
    serial_write_hex(frame->rip);

    serial_write("\r\nNYROX: RSP=");
    serial_write_hex(
        frame->vector == 8
            ? frame->rsp
            : (u64)&frame->rsp
    );

    serial_write("\r\nNYROX: RFLAGS=");
    serial_write_hex(frame->rflags);

    serial_write("\r\nNYROX: RAX=");
    serial_write_hex(frame->rax);
    serial_write("\r\nNYROX: RBX=");
    serial_write_hex(frame->rbx);
    serial_write("\r\nNYROX: RCX=");
    serial_write_hex(frame->rcx);
    serial_write("\r\nNYROX: RDX=");
    serial_write_hex(frame->rdx);

    serial_write("\r\nNYROX: SYSTEM HALTED\r\n");

    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static void inspect_memory_map(const NyroxBootInfo *info) {
    serial_write("NYROX: MEMORY MAP CHECK\r\n");

    if (!info->memory_map ||
        !info->memory_map_size ||
        info->memory_map_descriptor_size < sizeof(NyroxMemoryDescriptor) ||
        info->memory_map_size % info->memory_map_descriptor_size) {
        serial_write("NYROX: MEMORY MAP INVALID\r\n");
        return;
    }

    const u8 *cursor = (const u8 *)info->memory_map;
    u64 offset = 0;
    u64 conventional_pages = 0;
    u64 conventional_regions = 0;
    u64 descriptors = 0;

    while (offset < info->memory_map_size) {
        const NyroxMemoryDescriptor *entry =
            (const NyroxMemoryDescriptor *)(cursor + offset);

        if (entry->type == 7) {
            if (entry->number_of_pages > ~0ULL - conventional_pages) {
                serial_write("NYROX: MEMORY COUNT OVERFLOW\r\n");
                return;
            }

            serial_write("NYROX: FREE REGION BASE=");
            serial_write_hex(entry->physical_start);
            serial_write(" PAGES=");
            serial_write_hex(entry->number_of_pages);
            serial_write("\r\n");

            conventional_pages += entry->number_of_pages;
            ++conventional_regions;
        }

        ++descriptors;
        offset += info->memory_map_descriptor_size;
    }

    serial_write("NYROX: DESCRIPTORS=");
    serial_write_hex(descriptors);
    serial_write("\r\nNYROX: CONVENTIONAL REGIONS=");
    serial_write_hex(conventional_regions);
    serial_write("\r\nNYROX: CONVENTIONAL PAGES=");
    serial_write_hex(conventional_pages);
    serial_write("\r\n");
}

static u16 read_cs(void) {
    u16 value;
    __asm__ volatile ("mov %%cs, %0" : "=r"(value));
    return value;
}

void __attribute__((ms_abi))
kernel_main(NyroxBootInfo *boot_info) {
    serial_init();
    __asm__ volatile ("cli" : : : "memory");
    gdt_init();
    serial_write("NYROX: GDT LOADED\\r\\n");
    serial_write("NYROX: TR=");
    serial_write_hex(gdt_read_tr());
    serial_write("\\r\\n");
    idt_init();
    serial_write("NYROX: IDT LOADED\r\n");
    serial_write("NYROX: CS=");
    serial_write_hex(read_cs());
    serial_write("\r\n");
    serial_write("NYROX: KERNEL ENTERED\r\n");
    if (!boot_info ||
        boot_info->structure_version != NYROX_BOOT_INFO_VERSION) {
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }

    serial_write("NYROX: BOOT INFO OK\r\n");
    inspect_memory_map(boot_info);

    if (mm_init(boot_info)) {
        serial_write("NYROX: MEMORY MANAGER INIT OK\r\n");

        u64 free_before = mm_free_pages_count();
        serial_write("NYROX: FREE PAGES BEFORE=");
        serial_write_hex(free_before);
        serial_write("\r\n");

        u64 test_page = mm_alloc_page();

        if (test_page) {
            serial_write("NYROX: ALLOCATED PAGE=");
            serial_write_hex(test_page);
            serial_write("\r\n");

            if (mm_free_page(test_page))
                serial_write("NYROX: PAGE FREE OK\r\n");
            else
                serial_write("NYROX: PAGE FREE FAILED\r\n");

            u64 free_after = mm_free_pages_count();
            serial_write("NYROX: FREE PAGES AFTER=");
            serial_write_hex(free_after);
            serial_write("\r\n");

            if (free_before == free_after)
                serial_write("NYROX: ALLOCATOR TEST PASSED\r\n");
            else
                serial_write("NYROX: ALLOCATOR COUNT MISMATCH\r\n");
        } else {
            serial_write("NYROX: PAGE ALLOCATION FAILED\r\n");
        }
    } else {
        serial_write("NYROX: MEMORY MANAGER INIT FAILED\r\n");
    }

    if (console_init(boot_info) == 0) console_write("NYROX PC BOOTED\nCONSOLE READY");
    serial_write("NYROX: FRAMEBUFFER DRAW COMPLETE\r\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
