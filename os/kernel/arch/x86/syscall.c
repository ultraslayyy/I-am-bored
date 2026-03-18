#include <fs/fd.h> 
#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <lib/stdlib.h>
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
        case 0x00: // putchar
            put_char((char)r->ebx, DEFAULT_ATTR);
            break;
        case 0x01: // puts
            put_string((char *)r->ebx, DEFAULT_ATTR);
            break;
        case 0x02: // exit
            put_string("\n[process exited]\n", DEFAULT_ATTR);
            while (1) {
                asm volatile("hlt");
            }
            break;
        case 0x03: // getpid
        case 0x04: // fork
        case 0x05: // exec
        case 0x06: // wait
        case 0x07: // sleep
            break;
        case 0x10: // open
            r->eax = fd_open((char *)r->ebx);
            break;
        case 0x11: // close
            fd_close(r->ebx);
            r->eax = 0;
            break;
        case 0x12: // read
            r->eax = fd_read(r->ebx, (char *)r->ecx, r->edx);
            break;
        case 0x13: // write
            r->eax = fd_write(r->ebx, (const char *)r->ecx, r->edx);
            break;
        case 0x14: { // lseek
            int fd = r->ebx;
            size_t offset = r->ecx;
            if (fd >= 0 && fd < MAX_FD && fd_table[fd].used) {
                if (offset > fd_table[fd].node->size) {
                    offset = fd_table[fd].node->size;
                }
                fd_table[fd].offset = offset;
                r->eax = offset;
            } else {
                r->eax = -1;
            }
            break;
        }
        case 0x20: // mkdir
            fs_node_t *node = vfs_create((char *)r->ebx, FS_DIR);
            r->eax = node ? 0 : -1;
            break;
        case 0x21: // rmdir
            r->eax = fd_remove((char *)r->ebx, r->ecx);
            break;
        case 0x30: // get_time
        case 0x31: // uname
            break;
        default:
            put_char('[unknown syscall]', DEFAULT_ATTR);
            break;
    }
}