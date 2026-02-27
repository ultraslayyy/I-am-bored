global irq0
global irq1
extern isr_handler

section .text

irq0:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    mov eax, 32
    push eax

    call isr_handler

    add esp, 8
    mov esp, eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

irq1:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    mov eax, 33
    push eax
    
    call isr_handler

    add esp, 8
    mov esp, eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret
