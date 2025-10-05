[bits 32]
global _start
extern kernel_main

section .multiboot
    align 4
    dd 0x1BADB002
    dd 0
    dd -(0x1BADB002 + 0)

section .text
_start:
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite
