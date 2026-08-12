#include <arch.h>

int cmd_shutdown(int argc, char **argv) {
    (void)argc;
    (void)argv;

    acpi_shutdown();
    outw(0x604, 0x2000); // QEMU backup in case of incorrect flags
    return 0;
}

// Move into centralised reboot logic later
// and out of the command only
static void reboot_kbc(void) {
    while(inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

static void reboot_triple_fault(void) {
    asm volatile("cli");

    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idt = {0, 0};

    asm volatile("lidt %0" : : "m"(idt));
    asm volatile("int $0x03");
}

int cmd_reboot(int argc, char **argv) {
    (void)argc;
    (void)argv;

    acpi_reboot();

    for (volatile int i = 0; i < 100000; ++i);

    reboot_kbc();

    for (volatile int i = 0; i < 100000; ++i);

    reboot_triple_fault();

    for (;;);
}