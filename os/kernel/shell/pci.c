#include <drivers/pci/pci.h>
#include <lib/string.h>
#include <io/kernel_io.h>

void list_pci(void) {
    for (int slot = 0; slot < 32; slot++) {
        uint16_t vendor = pci_read_word(0, slot, 0, 0x00);
        if (vendor != 0xFFFF) {
            uint16_t device = pci_read_word(0, slot, 0, 0x02);

            char text[128];
            snprintf(text, sizeof(text), "PCI %d: vendor=%x device=%x\n", slot, vendor, device);
            put_string(text, DEFAULT_ATTR);
        }
    }
}

int cmd_pci(int argc, char **argv) {
    if (argc == 2) {
        list_pci();
        return 0;
    }

    put_string("Usage: pci list\n", DEFAULT_ATTR);
    return 1;
}