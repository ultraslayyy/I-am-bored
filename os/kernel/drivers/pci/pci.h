#pragma once
#include <lib/stdint.h>

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t  pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void pci_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
int pci_find_device(uint16_t vendor, uint16_t device, uint8_t *out_bus, uint8_t *out_slot, uint8_t *out_func);
int pci_find_class(uint8_t class_code, uint8_t subclass, uint8_t prog_if, uint8_t *out_bus, uint8_t *out_slot, uint8_t *out_func);