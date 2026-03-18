[bits 64]
global _start
extern multiboot2_parse
extern kernel_main
extern kernel_stack_end

section .multiboot
align 8
header_start:
    dd 0xE85250D6
    dd 0
    dd header_end - header_start
    dd -(0xE85250D6 + (header_end - header_start))
    align 8
framebuffer_tag_start:
    dw 5
    dw 1
    dd 20
    dd 0
    dd 0
    dd 32
framebuffer_tag_end:

    align 8
    dw 0
    dw 0
    dd 8
header_end:

section .text
_start:
    cli

    mov rsp, kernel_stack_end
    and rsp, -16
    sub rsp, 8

    mov edi, dword [rel multiboot_ptr]
    lea rsi, [rel boot_info]
    call multiboot2_parse

    lea rdi, [rel boot_info]
    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
boot_info:
    resb 4096

section .note.GNU-stack noalloc noexec nowrite