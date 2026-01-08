[bits 32]
global _start
extern kernel_main
extern kernel_stack_end

section .multiboot
    align 4
    dd 0x1BADB002
    dd 0
    dd -(0x1BADB002 + 0)

section .text
_start:
    mov esp, kernel_stack_end
    and esp, 0xFFFFFFF0          ; 16-byte alignment

    push ebx       ; multiboot_info*
    push eax       ; multiboot magic
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite
