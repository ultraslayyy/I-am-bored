#include "paging.h"
#include "user.h"

void enter_user_mode(void (*entry)()) {
    map_page(USER_STACK, USER_STACK, PAGE_PRESENT | PAGE_RW | PAGE_USER);

    asm volatile(
        "cli\n\t"
        "mov $0x23, %%ax\n\t" // User data selector
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"

        "pushl $0x23\n\t"       // SS
        "pushl $0x00801000\n\t" // ESP (top of user stack)
        "pushf\n\t"             // EFLAGS
        "pushl $0x1B\n\t"       // CS
        "pushl %0\n\t"          // EIP = entry
        "iret\n\t"
        :
        : "r"(entry)
    );
}