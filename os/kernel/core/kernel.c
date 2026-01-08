#include <stdint.h>
#include <stddef.h>
#include <io/kernel_io.h>
#include <shell/shell.h>
#include <fs/fs.h>
#include <drivers/memory/memory.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>

void kernel_main() {
    gdt_init();

    clear_screen();

    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    memory_init();
    fs_init();
    shell_init();
    
    idt_init();

    while (1) {
        __asm__ volatile("hlt");
    }
}
