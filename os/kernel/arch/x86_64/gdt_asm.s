global gdt_flush

section .text
[bits 64]

gdt_flush:
    lgdt [rdi]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    push 0x08
    lea rax, [rel .flush]
    push rax
    retfq

.flush:
    ret
