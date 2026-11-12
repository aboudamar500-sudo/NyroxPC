ENTRY(kernel_main)

SECTIONS
{
    . = 0x100000;

    .text : ALIGN(4096) {
        *(.text*)
    }

    . = ALIGN(4096);
    .rodata : {
        *(.rodata*)
    }

    . = ALIGN(4096);
    .data : {
        *(.data*)
    }

    . = ALIGN(4096);
    .bss : {
        *(COMMON)
        *(.bss*)
    }
}
