.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text

.global _start
.type _start, @function

_start:
    call kernel_main

hang:
    cli
    hlt
    jmp hang


.global keyboard_stub
.extern keyboard_handler
.type keyboard_stub, @function

keyboard_stub:
    pusha
    call keyboard_handler
    popa
    iret
