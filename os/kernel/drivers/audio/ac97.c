#include <drivers/pci/pci.h>
#include <drivers/serial/serial.h>
#include <lib/stdlib.h>
#include <arch.h>

#define AC97_VENDOR_ID 0x8086
#define AC97_DEVICE_ID 0x2415

#define AC97_CLASS    0x04
#define AC97_SUBCLASS 0x01
#define AC97_PROGIF   0x00

#define AC97_GLOB_CNT 0x2C
#define AC97_GLOB_STA 0x30

#define AC97_RESET         0x00
#define AC97_MASTER_VOLUME 0x02
#define AC97_PCM_VOLUME    0x18
#define AC97_EXTENDED_ID   0x28
#define AC97_POWERDOWN     0x26
#define AC97_VENDOR_ID1    0x7C
#define AC97_VENDOR_ID2    0x7E

#define AC97_PRIMARY_READY (1 << 8)

#define AC97_VOLUME_MAX 0x0000

#define AC97_BDL_ENTRIES 32

typedef struct {
    uint32_t address;
    uint32_t control_length;
} ac97_bdl_entry_t;

static ac97_bdl_entry_t *bdl;

static uint16_t nam_base  = 0;
static uint16_t nabm_base = 0;

static uint8_t ac97_bus;
static uint8_t ac97_slot;
static uint8_t ac97_func;

int ac97_initialised = 0;

static int ac97_codec_init(void) {
    serial_write("AC97: Resuming controller...\n");
    
    outl(nabm_base + AC97_GLOB_CNT, 0x02);

    uint32_t timeout = 1000000;
    while (!(inl(nabm_base + AC97_GLOB_STA) & AC97_PRIMARY_READY)) {
        if (--timeout == 0) {
            serial_write("AC97: Codec ready timeout\n");
            return 0;
        }
    }

    serial_write("AC97: Primary codec ready\n");

    outw(nam_base + AC97_RESET, 0x0000); // reset codec regs

    uint16_t capabilities = inw(nam_base + AC97_RESET);

    serial_write("AC97: Capabilities: ");
    serial_write_hex(capabilities);
    serial_write("\n");

    uint16_t vendor1 = inw(nam_base + AC97_VENDOR_ID1);
    uint16_t vendor2 = inw(nam_base + AC97_VENDOR_ID2);

    serial_write("AC97: Codec vendor ID 1: ");
    serial_write_hex(vendor1);
    serial_write("\n");

    serial_write("AC97: Codec vendor ID 2: ");
    serial_write_hex(vendor2);
    serial_write("\n");

    if ((vendor1 == 0xFFFF && vendor2 == 0xFFFF) || (vendor1 == 0x0000 && vendor2 == 0x0000)) {
        serial_write("AC97: Invalid codec identification\n");
        return 0;
    }

    // Unmute master and PCM, set to max vol
    outw(nam_base + AC97_MASTER_VOLUME, AC97_VOLUME_MAX);
    outw(nam_base + AC97_PCM_VOLUME, AC97_VOLUME_MAX);

    uint16_t extended = inw(nam_base + AC97_EXTENDED_ID); // extended capabilities

    serial_write("AC97: Extended capabilities: ");
    serial_write_hex(extended);
    serial_write("\n");

    serial_write("AC97: Codec initialised\n");

    return 1;
}

int ac97_init(void) {
    uint8_t bus, slot, func;

    serial_write("AC97: Initialising...");

    if (!pci_find_class(AC97_CLASS, AC97_SUBCLASS, AC97_PROGIF, &bus, &slot, &func)) {
        serial_write("AC97: Controller not found\n");
        return 0;
    }

    ac97_bus  = bus;
    ac97_slot = slot;
    ac97_func = func;

    uint16_t vendor = pci_read_word(bus, slot, func, 0x00);
    uint16_t device = pci_read_word(bus, slot, func, 0x02);

    if (vendor != AC97_VENDOR_ID || device != AC97_DEVICE_ID) {
        serial_write("AC97: Unsupported controller\n");
        return 0;
    }

    serial_write("AC97: Found Intel 82801AA\n");

    uint32_t bar0 = pci_read_dword(bus, slot, func, PCI_BAR0);
    uint32_t bar1 = pci_read_dword(bus, slot, func, PCI_BAR1);

    if (!(bar0 & 1) || !(bar1 & 1)) {
        serial_write("AC97: Invalid BAR configuration\n");
        return 0;
    }

    nam_base  = (uint16_t)(bar0 & ~3u);
    nabm_base = (uint16_t)(bar1 & ~3u);

    if (!nam_base || !nabm_base) {
        serial_write("AC97: Invalid BAR addresses\n");
        return 0;
    }

    serial_write("AC97: NAM base: ");
    serial_write_hex(nam_base);
    serial_write("\n");

    serial_write("AC97: NABM base: ");
    serial_write_hex(nabm_base);
    serial_write("\n");

    uint16_t command = pci_read_word(bus, slot, func, PCI_COMMAND);
    command |= PCI_COMMAND_IO | PCI_COMMAND_MASTER;
    pci_write_word(bus, slot, func, PCI_COMMAND, command);

    if (!ac97_codec_init()) {
        serial_write("AC97: Codec initialisation failed\n");
        return 0;
    }

    bdl = malloc(sizeof(ac97_bdl_entry_t) * AC97_BDL_ENTRIES);
    if (!bdl) {
        serial_write("Failed to allocate BDL\n");
        return 0;
    }

    ac97_initialised = 1;

    serial_write("AC97: Controller initialised\n");

    return 1;
}