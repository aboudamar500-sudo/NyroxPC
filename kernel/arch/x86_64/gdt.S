.section .text
.code64

.global gdt_load
.type gdt_load, @function

gdt_load:
    lgdt (%rdi)

    leaq 1f(%rip), %rax
    pushq $0x08
    pushq %rax
    lretq

1:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    movw $0x18, %ax
    ltr %ax
    ret

.section .note.GNU-stack,"",@progbits
