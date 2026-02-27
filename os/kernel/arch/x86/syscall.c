#include <fs/fs.h>
#include <io/kernel_io.h>
#include "idt.h"
#include "syscall.h"

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} regs_t;

extern void syscall_stub();
void syscall_handler(regs_t *r);


void syscall_init(void) {
    // Use interrupt 0x08 for syscalls
    idt_set_gate(128, (uint32_t)syscall_stub, 0x08, 0xEE);
}

/**
 * Current idea:
 * 0x00-0x0F - process / console
 * 0x10-0x1F - file / VFS
 * 0x20-0x2F - filesystem / directories
 * 0x30-0x3F - system info / time
 */
void syscall_handler(regs_t *r) {
    switch (r->eax) {
        case 0x00:
            put_char((char)r->ebx, DEFAULT_ATTR);
            break;
        case 0x01:
            put_string((char *)r->ebx, DEFAULT_ATTR);
            break;
        case 0x02:
            put_string("\n[process exited]\n", DEFAULT_ATTR);
            while (1) {
                asm volatile("hlt");
            }
            break;
        default:
            put_char('?', DEFAULT_ATTR);
            break;
    }
}