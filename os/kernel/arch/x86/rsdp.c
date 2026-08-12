#include <lib/string.h>
#include "rsdp.h"

#define RSDP_SIGNATURE "RSD PTR "

static int rsdp_checksum(void *addr, uint32_t length) {
    uint8_t sum = 0;
    for (uint32_t i = 0; i < length; ++i) {
        sum += ((uint8_t *)addr)[i];
    }
    return sum == 0;
}

static int rsdp_validate(void *ptr) {
    RSDP1 *rsdp = (RSDP1 *)ptr;

    if (memcmp(rsdp->Signature, RSDP_SIGNATURE, 8) != 0) {
        return 0;
    }

    // ACPI 1.0 checksum
    if (!rsdp_checksum(rsdp, 20)) {
        return 0;
    }

    // If ACPI 2.0+ validate extended part
    if (rsdp->Revision >= 2) {
        RSDP2 *rsdp2 = (RSDP2 *)ptr;

        if (!rsdp_checksum(rsdp2, rsdp2->Length)) {
            return 0;
        }
    }

    return 1;
}

static void *scan_range(uint32_t start, uint32_t end) {
    for (uint32_t addr = start; addr < end; addr += 16) {
        void *ptr = (void *)addr;

        if (memcmp(ptr, RSDP_SIGNATURE, 8) == 0) {
            if (rsdp_validate(ptr)) {
                return ptr;
            }
        }
    }
    
    return NULL;
}

void *find_rsdp(void) {
    uint16_t edba_seg = *(uint16_t *)0x40E;
    uint32_t edba_addr = ((uint32_t)edba_seg) << 4;

    void *rsdp = scan_range(edba_addr, edba_addr + 1024);
    if (rsdp) {
        return rsdp;
    }

    // Fallback to BIOS ROM
    return scan_range(0xE0000, 0x100000);
}

RSDP1 *rsdp_get(void) {
    return (RSDP1 *)find_rsdp();
}