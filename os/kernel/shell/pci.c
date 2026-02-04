#include <drivers/pci/pci.h>
#include <lib/string.h>
#include <lib/stddef.h>
#include <io/kernel_io.h>
#include <lib/stdlib.h>

void list_pci(void) {
    for (size_t slot = 0; slot < 32; slot++) {
        uint16_t vendor = pci_read_word(0, slot, 0, 0x00);
        if (vendor != 0xFFFF) {
            uint16_t device = pci_read_word(0, slot, 0, 0x02);

            char text[128];
            snprintf(text, sizeof(text), "PCI %d: vendor=%x device=%x\n", slot, vendor, device);
            put_string(text, DEFAULT_ATTR);
        }
    }
}

void pci_info(size_t slot) {
    uint8_t bus  = 0;
    uint8_t func = 0;

    uint16_t vendor = pci_read_word(0, slot, 0, 0x00);
    if (vendor == 0xFFFF) {
        put_string("No device present in that slot. Run 'pci list' to see all devices.\n", DEFAULT_ATTR);
        return;
    }

    uint16_t device  = pci_read_word(0, slot, 0, 0x02);
    uint16_t command = pci_read_word(0, slot, 0, 0x04);
    uint16_t status  = pci_read_word(0, slot, 0, 0x06);

    uint8_t revision = pci_read_byte(bus, slot, func, 0x08);
    uint8_t prog_if  = pci_read_byte(bus, slot, func, 0x09);
    uint8_t subclass = pci_read_byte(bus, slot, func, 0x0A);
    uint8_t class    = pci_read_byte(bus, slot, func, 0x0B);

    uint8_t header   = pci_read_byte(bus, slot, func, 0x0E);
    uint8_t irq_line = pci_read_byte(bus, slot, func, 0x3C);
    uint8_t irq_pin  = pci_read_byte(bus, slot, func, 0x3D);

    char buf[256];

    snprintf(buf, sizeof(buf),
        "PCI device at bus %u, slot %u, func %u\n"
        " Vendor ID:   0x%x\n"
        " Device ID:   0x%x\n"
        " Class:       0x%x\n"
        " Subclass:    0x%x\n"
        " Prog IF:     0x%x\n"
        " Revision:    0x%x\n"
        " Command:     0x%x\n"
        " Status:      0x%x\n"
        " Header Type: 0x%x\n"
        " IRQ line:    %u\n"
        " IRQ pin:     %u\n",
        bus, slot, func,
        vendor, device,
        class, subclass, prog_if, revision,
        command, status,
        header, irq_line, irq_pin
    );

    put_string(buf, DEFAULT_ATTR);
}

int cmd_pci(int argc, char **argv) {
    if (argc < 2) {
        put_string("Usage: pci <list|info>\n", DEFAULT_ATTR);
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        list_pci();
        return 0;
    } else if (strcmp(argv[1], "info") == 0) {
        if (argc != 3) {
            put_string("Usage: pci info <slot>\n", DEFAULT_ATTR);
        }

        size_t slot = (size_t)atoi(argv[2]);
        pci_info(slot);
        return 0;
    }

    put_string("Usage: pci <list|info>\n", DEFAULT_ATTR);
    return 1;
}