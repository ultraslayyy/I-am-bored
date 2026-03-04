#include <block/block.h>
#include <drivers/ata/ata.h>
#include <drivers/memory/memory.h>
#include <fs/fd.h>
#include <fs/ramfs.h>
#include <fs/vfs.h>
#include <fs/fat16/fat16.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include <shell/shell.h>
#include "boot_info.h"

extern uint32_t kernel_stack_end;

/* static void print_hex_byte(uint8_t value) {
    const char *hex = "0123456789ABCDEF";
    
    char out[3];
    out[0] = hex[(value >> 4) & 0xF];
    out[1] = hex[value & 0xF];
    out[2] = '\0';

    put_string(out, DEFAULT_ATTR);
} */

void kernel_main(boot_info_t *mbi) {
    gdt_init();
    tss_init((uint32_t)&kernel_stack_end);
    
    clear_screen();

    memory_init(mbi);
    paging_init();

    ata_init();
    vfs_init();
    // ramfs_init();
    block_device_t *dev = block_get_device();
    fat16_init(dev);
    fd_init();

    char path[MAX_PATH_LEN];
    vfs_get_path(kernel_cwd, path, sizeof(path));

    put_string(path, DEFAULT_ATTR);
    put_char(' ', DEFAULT_ATTR);
    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    idt_init();
    syscall_init();
    scheduler_init();

    pit_init(100);

    while (1) {
        __asm__ volatile("hlt");
    }
}