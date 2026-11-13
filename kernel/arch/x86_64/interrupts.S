.section .text
.code64

.extern exception_handler

.macro ISR_NOERR n
.global nyrox_isr_\n
.type nyrox_isr_\n, @function
nyrox_isr_\n:
    pushq $0
    pushq $\n
    jmp nyrox_isr_common
.endm

.macro ISR_ERROR n
.global nyrox_isr_\n
.type nyrox_isr_\n, @function
nyrox_isr_\n:
    pushq $\n
    jmp nyrox_isr_common
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERROR 8
ISR_NOERR 9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERROR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERROR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERROR 29
ISR_ERROR 30
ISR_NOERR 31

.global nyrox_default_stub
.type nyrox_default_stub, @function
nyrox_default_stub:
    cli
1:
    hlt
    jmp 1b

nyrox_isr_common:
    cld

    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rsp, %rdi

    andq $-16, %rsp
    call exception_handler

2:
    cli
    hlt
    jmp 2b

.section .note.GNU-stack,"",@progbits
