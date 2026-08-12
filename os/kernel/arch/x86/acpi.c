#include <lib/stdint.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include "io.h"

typedef struct {
    char Signature[4];
    uint32_t Length;
} __attribute__((packed)) ACPISDTHeader;

typedef struct {
    ACPISDTHeader h;
    uint32_t firmware_ctrl;
    uint32_t dsdt;

    uint8_t  reserved;

    uint8_t  preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;

    uint8_t  reserved2[10];

    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;

    // ACPI 2.0+
    uint8_t  reserved3[40];

    struct {
        uint8_t  address_space;
        uint8_t  bit_width;
        uint8_t  bit_offset;
        uint8_t  access_size;
        uint64_t address;
    } __attribute__((packed)) reset_reg;

    uint8_t  reset_value;
} __attribute__((packed)) FADT;

static uint16_t SLP_TYPa = 0;
static uint16_t SLP_TYPb = 0;
static uint16_t PM1a_CNT = 0;
static uint16_t PM1b_CNT = 0;

static uint8_t  reset_value = 0;
static uint64_t reset_address = 0;
static uint8_t  reset_space = 0;

static void *copy_table_phys(uint64_t phys_addr) {
    ACPISDTHeader *header = (ACPISDTHeader *)(uintptr_t)phys_addr;
    uint32_t length = header->Length;

    void *buf = malloc(length);
    memcpy(buf, (void *)(uintptr_t)phys_addr, length);
    return buf;
}

void *acpi_find_table(ACPISDTHeader *sdt, const char *sig, int use_xsdt) {
    ACPISDTHeader *h = (ACPISDTHeader *)sdt;

    if (use_xsdt) {
        uint64_t *entries = (uint64_t *)((uint8_t *)h + sizeof(ACPISDTHeader));
        int count = (h->Length - sizeof(ACPISDTHeader)) / 8;

        for (int i = 0; i < count; ++i) {
            if (!entries[i]) continue;

            ACPISDTHeader *tbl = copy_table_phys(entries[i]);

            if (memcmp(tbl->Signature, sig, 4) == 0) {
                return tbl;
            }
        }
    } else {
        uint32_t *entries = (uint32_t *)((uint8_t *)h + sizeof(ACPISDTHeader));
        int count = (h->Length - sizeof(ACPISDTHeader)) / 4;

        for (int i = 0; i < count; ++i) {
            if (!entries[i]) continue;

            ACPISDTHeader *tbl = copy_table_phys(entries[i]);

            if (memcmp(tbl->Signature, sig, 4) == 0) {
                return tbl;
            }
        }
    }

    return NULL;
}

int acpi_parse_s5(uint8_t *dsdt, uint32_t length) {
    for (uint32_t i = 0; i < length - 4; ++i) {
        if (memcmp(dsdt + i, "_S5_", 4) == 0) {
            uint8_t *pkg = dsdt + i;

            // Skip NameOp (0x08) if present
            if (pkg[-1] == 0x08) {
                pkg += 5;
            }

            // Expect PackageOp (0x12)
            if (*pkg != 0x12) {
                continue;
            }

            pkg++;

            // Skip package length (AML encoding)
            uint8_t len = *pkg & 0x3F;
            (void)len;
            pkg++;
            // Skip element count
            pkg++;

            // First value (SLP_TYPa)
            if (*pkg == 0x0A) {
                pkg++;
            }
            SLP_TYPa = (*pkg) << 10; // BytePrefix
            pkg++;

            // Second value (SLP_TYPb)
            if (*pkg == 0x0A) {
                pkg++;
            }
            SLP_TYPb = (*pkg) << 10;

            return 1;
        }
    }

    return 0;
}

static int acpi_use_xsdt(void *rsdp_ptr) {
    uint8_t revision = *((uint8_t *)rsdp_ptr + 15);
    if (revision < 2) return 0;

    uint64_t xsdt = *(uint64_t *)((uint8_t *)rsdp_ptr + 24);

    if (xsdt == 0) return 0;

    if ((xsdt & 0xFFF) != 0) return 0;

    return 1;
}

int acpi_init(void *rsdp_ptr) {
    int use_xsdt = acpi_use_xsdt(rsdp_ptr);

    uint64_t sdt_addr;
    ACPISDTHeader *sdt;

    if (use_xsdt) {
        sdt_addr = *(uint64_t *)((uint8_t *)rsdp_ptr + 24);
    } else {
        sdt_addr = *(uint32_t *)((uint8_t *)rsdp_ptr + 16);
    }

    sdt = copy_table_phys(sdt_addr);

    // Find FADT
    FADT *fadt = (FADT *)acpi_find_table(sdt, "FACP", use_xsdt);
    if (!fadt) {
        return 0;
    }

    if (fadt->smi_cmd_port && fadt->acpi_enable) {
        outb(fadt->smi_cmd_port, fadt->acpi_enable);
    }

    reset_address = fadt->reset_reg.address;
    reset_value   = fadt->reset_value;
    reset_space   = fadt->reset_reg.access_size;

    PM1a_CNT = (uint16_t)fadt->pm1a_cnt_blk;
    PM1b_CNT = (uint16_t)fadt->pm1b_cnt_blk;

    // Get DSDT
    ACPISDTHeader *dsdt_hdr = copy_table_phys(fadt->dsdt);
    uint8_t *dsdt = (uint8_t *)dsdt_hdr;

    if (!acpi_parse_s5(dsdt, dsdt_hdr->Length)) {
        return 0;
    }

    return 1;
}

void acpi_shutdown(void) {
    uint16_t SLP_EN = 1 << 13;

    if (PM1a_CNT) {
        outw(PM1a_CNT, SLP_TYPa | SLP_EN);
    }

    if (PM1b_CNT) {
        outw(PM1b_CNT, SLP_TYPb | SLP_EN);
    }
}

void acpi_reboot(void) {
    if (reset_address != 0) {
        if (reset_space == 1) {
            outb((uint16_t)reset_address, reset_value);
        }
    }
}