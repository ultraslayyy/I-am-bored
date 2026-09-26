#include <arch.h>
#include <drivers/pci/pci.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include "ehci.h"

#define EHCI_CLASS    0x0C
#define EHCI_SUBCLASS 0x03
#define EHCI_PROGIF   0x02

#define EHCI_USBCMD     0x20
#define EHCI_USBSTS     0x24
#define EHCI_CONFIGFLAG 0x40
#define EHCI_PORTSC     0x44

static uint32_t mmio_base;
static uint32_t op_base;

static volatile uint32_t *reg(uint32_t offset) {
    return (volatile uint32_t *)(op_base + offset);
}

int ehci_init(void) {
    uint8_t bus, slot, func;

    if (!pci_find_class(EHCI_CLASS, EHCI_SUBCLASS, EHCI_PROGIF, &bus, &slot, &func)) {
        return 0;
    }

    uint32_t bar0 = pci_read_dword(bus, slot, func, 0x10);
    mmio_base = bar0 & ~0xF;

    map_page(mmio_base, mmio_base, PAGE_PRESENT | PAGE_RW);

    uint8_t cap_len = *(volatile uint8_t *)(mmio_base);
    op_base = mmio_base + cap_len;

    char buf[64];
    snprintf(buf, sizeof(buf), "EHCI @ %x (op @ %x)\n", mmio_base, op_base);
    put_string(buf, DEFAULT_ATTR);

    *reg(EHCI_USBCMD) |= (1 << 1);
    while (*reg(EHCI_USBCMD) & (1 << 1));

    *reg(EHCI_USBCMD) |= (1 << 0);

    *reg(EHCI_CONFIGFLAG) = 1;

    return 1;
}

void ehci_probe_ports(void) {
    put_string("Probing ports...\n", DEFAULT_ATTR);

    for (int i = 0; i < 8; ++i) {
        volatile uint32_t *port = (uint32_t *)(op_base + EHCI_PORTSC + i * 4);
        uint32_t status = *port;

        if (status & 1) {
            char buf[64];
            snprintf(buf, sizeof(buf), "Device of port %d\n", i);
            put_string(buf, DEFAULT_ATTR);

            *port |= (1 << 8);
            for (volatile int d = 0; d < 1000000; ++d);
            *port &= ~(1 << 8);

            snprintf(buf, sizeof(buf), "Port %d reset\n", i);
            put_string(buf, DEFAULT_ATTR);
        }
    }
}