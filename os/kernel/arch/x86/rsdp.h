#pragma once
#include <lib/stdint.h>

typedef struct {
    char Signature[8];    // "RSD PTR "
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;     // 0 = ACPI 1.0, 2 = ACPI 2.0+
    uint32_t RsdtAddress; // 32-bit physical address
} __attribute__((packed)) RSDP1;

typedef struct {
    RSDP1 first;
    uint32_t Length;
    uint64_t XsdtAddress; // 64-bit
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed)) RSDP2;

RSDP1 *rsdp_get(void);