/**
 * Maybe this would also work and is another way I can do the syscalls (putting here so I don't forget)
 * 
 * asm volatile(
 *     "int $0x80"
 *     :
 *     : "a"(0x01), "b"(msg)
 *     : "memory"
 * );
 * 
 * asm volatile(
 *     "int $0x80"
 *     :
 *     : "a"(0x02)
 * );
 * 
 */

void user_main() {
    const char *msg = "Hello World!\n";

    asm volatile(
        "mov $0x01, %%eax\n"   // 1 = write_str
        "mov %0, %%ebx\n "    // str to write
        "int $0x80\n"
        :
        : "r"(msg)
        : "eax", "ebx"
    );

    asm volatile(
        "mov $0x02, %%eax\n"   // 2 = exit
        "int $0x80\n"
        :
        :
        : "eax"
    );

    while(1) {
        asm volatile("hlt");
    }
}