#include <drivers/memory/memory.h>
#include <fs/fd.h>
#include <fs/ramfs.h>
#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <shell/shell.h>
#include "boot_info.h"

extern uint32_t kernel_stack_end;

// void taskA();
// void taskB();

/* void user_main() {
    while (1) {
        asm volatile("int $0x80");
    }
} */

void kernel_main(boot_info_t *mbi) {
    gdt_init();
    tss_init((uint32_t)&kernel_stack_end);
    
    clear_screen();

    memory_init(mbi);
    paging_init();

    vfs_init();
    ramfs_init();
    vfs_mount_root(&ramfs);
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
    // task_create(taskA); 
    // task_create(taskB);

    // enter_user_mode(user_main);

    while (1) {
        __asm__ volatile("hlt");
    }
}

/* void taskA() {
    while (1) {
        put_string("A", DEFAULT_ATTR);
        for(volatile int i = 0; i < 1000000; ++i); // Slow down for visibility
    }
}

void taskB() {
    while (1) {
        put_string("B", DEFAULT_ATTR);
        for(volatile int i = 0; i < 1000000; ++i); // Slow down for visibility
    }
} */