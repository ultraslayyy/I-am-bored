#include <io/kernel_io.h>
#include "idt.h"
#include "syscall.h"

extern void syscall_stub();
void syscall_handler();

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} regs_t;

void syscall_init(void) {
    // Use interrupt 0x08 for syscalls
    idt_set_gate(128, (uint32_t)syscall_stub, 0x08, 0xEE);
}

void syscall_handler() {
    // Example
    put_char('S', DEFAULT_ATTR);
}