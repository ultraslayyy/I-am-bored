#include <io/kernel_io.h>
#include <shell/shell.h>
#include <fs/fs.h>
#include <drivers/memory/memory.h>
#include <fs/cwd.h>
#include "boot_info.h"

// void taskA();
// void taskB();

void kernel_main(boot_info_t *mbi) {
    gdt_init();
    clear_screen();

    put_string(g_cwd, DEFAULT_ATTR);
    put_char(' ', DEFAULT_ATTR);
    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    memory_init(mbi);
    fs_init();

    idt_init();
    scheduler_init();

    pit_init(100);
    // task_create(taskA); 
    // task_create(taskB);

    while (1) {
        __asm__ volatile("hlt");
    }
}

/* void taskA() {
    while (1) {
        put_string("A", DEFAULT_ATTR);
        for(volatile int i = 0; i < 1000000; i++); // Slow down for visibility
    }
}

void taskB() {
    while (1) {
        put_string("B", DEFAULT_ATTR);
        for(volatile int i = 0; i < 1000000; i++); // Slow down for visibility
    }
} */