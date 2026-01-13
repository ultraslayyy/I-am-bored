#include <lib/stdint.h>
#include <io/kernel_io.h>
#include <shell/shell.h>
#include <fs/fs.h>
#include <drivers/memory/memory.h>

void kernel_main(uint32_t magic, struct multiboot_info* mbi) {
    if (magic != MULTIBOOT_MAGIC) {
        for (;;);
    }

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
