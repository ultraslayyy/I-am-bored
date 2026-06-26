#include "paging.h"
#include "user.h"

void enter_user_mode(void (*entry)()) {
    uint32_t user_esp = 0x500000;

    asm volatile(
        "cli\n\t"
        "mov $0x23, %%ax\n\t" // User data selector
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"

        "pushl $0x23\n\t"      // SS
        "pushl $0x4FFFFC\n\t"  // ESP (top of user stack)
        "pushfl\n\t"           // EFLAGS
        "pop %%eax\n\t"
        "or $0x200, %%eax\n\t" // enable interupts
        "push %%eax\n\t"

        "pushl $0x1B\n\t"      // CS
        "pushl %0\n\t"         // EIP = entry
        "iret\n\t"
        :
        : "r"(entry)
        : "eax"
    );
}