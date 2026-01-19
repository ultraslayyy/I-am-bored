#include <io/kernel_io.h>
#include <shell/shell.h>
#include <fs/fs.h>
#include <drivers/memory/memory.h>
#include "boot_info.h"

#include <lib/string.h>
void kernel_main(boot_info_t *mbi) {
    gdt_init();
    clear_screen();

    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    memory_init(mbi);

    fs_init();
    shell_init();
    idt_init();

    while (1) {
        __asm__ volatile("hlt");
    }
}
