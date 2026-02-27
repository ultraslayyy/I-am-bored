#include <io/kernel_io.h>
#include <shell/shell.h>
#include <fs/fs.h>
#include <drivers/memory/memory.h>
#include <fs/cwd.h>
#include "boot_info.h"

extern uint32_t kernel_stack_end;

void user_main() {
    // write 'H'
    asm volatile(
        "mov $0x00, %%eax\n"   // syscall 0 = write_char
        "mov $'H', %%ebx\n"    // char to write
        "int $0x80\n"
        :
        :
        : "eax", "ebx"
    );

    // exit
    asm volatile(
        "mov $0x02, %%eax\n"   // syscall 2 = exit
        "int $0x80\n"
        :
        :
        : "eax"
    );

    while(1) { asm volatile("hlt"); }
}

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

    put_string(g_cwd, DEFAULT_ATTR);
    put_char(' ', DEFAULT_ATTR);
    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    memory_init(mbi);
    paging_init();
    fs_init();

    idt_init();
    syscall_init();
    scheduler_init();

    pit_init(100);
    // task_create(taskA); 
    // task_create(taskB);

    enter_user_mode(user_main);

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