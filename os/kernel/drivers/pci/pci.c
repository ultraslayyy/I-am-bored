#include "pci.h"
#include <arch.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint32_t)(
        (1 << 31) |
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC)
    );
}

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, pci_address(bus, slot, func, offset));
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t value = pci_read_dword(bus, slot, func, offset);
    return (value >> ((offset & 2) * 8)) & 0xFFFF;
}

uint8_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t value = pci_read_dword(bus, slot, func, offset);
    return (value >> ((offset & 3) * 8)) & 0xFF;
}

void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    outl(PCI_CONFIG_ADDRESS, pci_address(bus, slot, func, offset));
    outl(PCI_CONFIG_DATA, value);
}

void pci_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value) {
    uint32_t old = pci_read_dword(bus, slot, func, offset);
    uint32_t shift = (offset & 2) * 8;
    uint32_t mask = 0xFFFF << shift;
    uint32_t newv = (old & ~mask) | ((uint32_t)value << shift);
    pci_write_dword(bus, slot, func, offset, newv);
}

int pci_find_device(uint16_t vendor, uint16_t device, uint8_t *out_bus, uint8_t *out_slot, uint8_t *out_func) {
    for (uint8_t bus = 0; bus < 1; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            uint16_t v = pci_read_word(bus, slot, 0, 0x00);
            if (v == 0xFFFF) continue;

            uint16_t d = pci_read_word(bus, slot, 0, 0x02);
            if (v == vendor && d == device) {
                *out_bus = bus;
                *out_slot = slot;
                *out_func = 0;
                return 1;
            }
        }
    }
    return 0;
}
