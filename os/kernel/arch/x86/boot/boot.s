[bits 32]
global _start

extern kernel_main
extern kernel_stack_end
extern multiboot2_parse

section .multiboot
align 8
header_start:
    dd 0xE85250D6
    dd 0
    dd header_end - header_start
    dd -(0xE85250D6 + 0 + (header_end - header_start))

    align 8
framebuffer_tag_start:
    dw 5
    dw 0
    dd 20
    ; dd 1920
    ; dd 1080
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
    mov esp, kernel_stack_end
    and esp, 0xFFFFFFF0          ; 16-byte alignment

    lea eax, [boot_info]
    push eax
    mov eax, ebx
    push eax

    call multiboot2_parse
    add esp, 8

    lea eax, [boot_info]
    push eax
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
boot_info:
    resb 4096

section .note.GNU-stack noalloc noexec nowrite
