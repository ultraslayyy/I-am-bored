#include <arch.h>
#include <drivers/pci/pci.h>
#include <drivers/serial/serial.h>
#include <io/kernel_io.h>
#include <lib/stdint.h>
#include "uhci.h"

static uhci_controller_t controller = {0};
static uint32_t uhci_frame_list[UHCI_FRAME_COUNT] __attribute__((aligned(4096)));
static uint8_t uhci_setup_packet[8] __attribute__((aligned(16)));
static uint8_t uhci_device_descriptor[18] __attribute__((aligned(16)));
static uhci_td_t uhci_setup_td __attribute__((aligned(16)));
static uhci_td_t uhci_data_td  __attribute__((aligned(16)));
static uhci_td_t uhci_status_td __attribute__((aligned(16)));

// TODO: at least 10ms timeout
static void uhci_delay(void) {
    for (volatile uint32_t i = 0; i < 100000; ++i);
}

static int uhci_find_controller(uint16_t *io_base) {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;

    if (!pci_find_class(UHCI_PCI_CLASS, UHCI_PCI_SUBCLASS, UHCI_PCI_PROGIF, &bus, &slot, &func)) {
        return 1;
    }

    uint32_t bar4 = pci_read_dword(bus, slot, func, 0x20);

    if (!(bar4 & 1)) {
        return 1;
    }

    *io_base = (uint16_t)(bar4 & 0xFFFC);

    serial_write("UHCI: PCI device at ");
    serial_write_dec((uint32_t)bus);
    serial_write(".");
    serial_write_dec((uint32_t)slot);
    serial_write(".");
    serial_write_dec((uint32_t)func);
    serial_write("\n");

    serial_write("UHCI: BAR4 = ");
    serial_write_hex((uint64_t)bar4);
    serial_write(", I/O base = ");
    serial_write_hex((uint64_t)*io_base);
    serial_write("\n");

    return 0;
}

uint16_t uhci_read16(uint16_t reg) {
    return inw(controller.io_base + reg);
}

uint32_t uhci_read32(uint16_t reg) {
    uint32_t low;
    uint32_t high;

    low  = inw(controller.io_base + reg);
    high = inw(controller.io_base + reg + 2);

    return low | (high << 16);
}

void uhci_write16(uint16_t reg, uint16_t value) {
    outw(controller.io_base + reg, value);
}

void uhci_write32(uint16_t reg, uint32_t value) {
    outw(controller.io_base + reg, (uint16_t)(value & 0xFFFF));
    outw(controller.io_base + reg + 2, (uint16_t)(value >> 16));
}

int uhci_reset(void) {
    unsigned int timeout;

    uhci_write16(UHCI_REG_USBCMD, uhci_read16(UHCI_REG_USBCMD) & ~UHCI_CMD_RS);

    for (timeout = 0; timeout < 100000; ++timeout) {
        if (uhci_read16(UHCI_REG_USBSTS) & UHCI_STS_HCH) {
            break;
        }
    }

    uhci_write16(UHCI_REG_USBCMD, UHCI_CMD_HCRESET);

    for (timeout = 0; timeout < 100000; ++timeout) {
        if (!(uhci_read16(UHCI_REG_USBCMD) & UHCI_CMD_HCRESET)) {
            break;
        }
    }

    if (uhci_read16(UHCI_REG_USBCMD) & UHCI_CMD_HCRESET) {
        serial_write("UHCI: controller reset timed out\n");
        return 1;
    }

    uhci_write16(UHCI_REG_USBINTR, 0);
    uhci_write16(UHCI_REG_USBSTS, UHCI_STS_USBINT | UHCI_STS_ERROR | UHCI_STS_RD | UHCI_STS_HSE | UHCI_STS_HCPE);

    uhci_write16(UHCI_REG_USBCMD, UHCI_CMD_CONFIG | UHCI_CMD_MAXP);

    return 0;
}

static uint16_t uhci_get_port(unsigned int port) {
    if (port == 1) {
        return uhci_read16(UHCI_REG_PORTSC1);
    }
    
    if (port == 2) {
        return uhci_read16(UHCI_REG_PORTSC2);
    }

    return 0;
}

void uhci_dump_ports(void) {
    uint16_t port1;
    uint16_t port2;

    port1 = uhci_get_port(1);
    port2 = uhci_get_port(2);

    serial_write("UHCI: port 1 = ");
    serial_write_dec((uint32_t)port1);
    serial_write("\n");

    serial_write("UHCI: port 2 = ");
    serial_write_dec((uint32_t)port2);
    serial_write("\n");

    serial_write("UHCI: port 1 device: ");
    if (port1 & UHCI_PORT_CCS) {
        serial_write("connected");
    } else {
        serial_write("not connected");
    }
    serial_write("\n");
    
    serial_write("UHCI: port 2 device: ");
    if (port2 & UHCI_PORT_CCS) {
        serial_write("connected");
    } else {
        serial_write("not connected");
    }
    serial_write("\n");

    if (port1 & UHCI_PORT_LSDA) {
        serial_write("UHCI: port 1 is low-speed\n");
    }

    if (port2 & UHCI_PORT_LSDA) {
        serial_write("UHCI: port 2 is low-speed\n");
    }
}

int uhci_reset_port(unsigned int port) {
    uint16_t reg;
    uint16_t value;

    if (port == 1) {
        reg = UHCI_REG_PORTSC1;
    } else if (port == 2) {
        reg = UHCI_REG_PORTSC2;
    } else {
        return 1;
    }

    value = uhci_read16(reg);

    if (!(value & UHCI_PORT_CCS)) {
        serial_write("UHCI: port ");
        serial_write_dec((uint32_t)port);
        serial_write(" has no device\n");

        return 1;
    }

    value |= UHCI_PORT_RESET;
    uhci_write16(reg, value);

    uhci_delay();

    value  = uhci_read16(reg);
    value &= ~UHCI_PORT_RESET;
    uhci_write16(reg, value);

    uhci_delay();

    value  = uhci_read16(reg);
    value |= UHCI_PORT_PE;
    uhci_write16(reg, value);

    value = uhci_read16(reg);

    if (!(value & UHCI_PORT_PE)) {
        serial_write("UHCI: failed to enable port ");
        serial_write_dec((uint32_t)port);
        serial_write("\n");

        return 1;
    }

    serial_write("UHCI: port ");
    serial_write_dec((uint32_t)port);
    serial_write(" reset and enabled\n");

    return 0;
}

static uint32_t uhci_make_token(uint8_t pid, uint8_t device, uint8_t endpoint, uint8_t toggle, uint16_t max_length) {
    uint32_t token = 0;

    token |= pid;
    token |= ((uint32_t)device   & 0x7F) << 8;
    token |= ((uint32_t)endpoint & 0x0F) << 15;
    token |= ((uint32_t)toggle   & 0x01) << 19;

    if (max_length == 0) {
        token |= 0x7FF << 21;
    } else {
        token |= ((uint32_t)(max_length - 1) & 0x7FF) << 21;
    }

    return token;
}

static void uhci_make_get_descriptor_setup(void) {
    uhci_setup_packet[0] = 0x80;
    uhci_setup_packet[1] = 0x06;
    uhci_setup_packet[2] = 0x00;
    uhci_setup_packet[3] = 0x01;
    uhci_setup_packet[4] = 0x00;
    uhci_setup_packet[5] = 0x00;
    uhci_setup_packet[6] = 18;
    uhci_setup_packet[7] = 0x00;
}

static void uhci_init_td(uhci_td_t *td, uint32_t link, uint32_t token, uint32_t buffer) {
    td->link = link;
    td->status = UHCI_TD_ACTIVE | UHCI_TD_C_ERR_MASK;
    td->token = token;
    td->buffer = buffer;

    td->reserved[0] = 0;
    td->reserved[1] = 0;
    td->reserved[2] = 0;
    td->reserved[3] = 0;
}

static void uhci_setup_get_descriptor(void) {
    uhci_make_get_descriptor_setup();

    uhci_init_td(&uhci_setup_td, (uint32_t)&uhci_data_td, uhci_make_token(UHCI_PID_SETUP, 0, 0, 0, 8), (uint32_t)uhci_setup_packet);
    uhci_init_td(&uhci_data_td, (uint32_t)&uhci_status_td, uhci_make_token(UHCI_PID_IN, 0, 0, 1, 18), (uint32_t)uhci_device_descriptor);
    uhci_init_td(&uhci_status_td, UHCI_LINK_TERMINATE, uhci_make_token(UHCI_PID_IN, 0, 0, 1, 0), (uint32_t)0);
}

static int uhci_td_failed(const uhci_td_t *td) {
    uint32_t status = td->status;

    if (status & UHCI_TD_STALLED) {
        serial_write("UHCI: TD stalled\n");
        return 1;
    }
    if (status & UHCI_TD_DBUFFER) {
        serial_write("UHCI: TD data-buffer error\n");
        return 1;
    }

    if (status & UHCI_TD_BABBLE) {
        serial_write("UHCI: TD babble\n");
        return 1;
    }

    if (status & UHCI_TD_CRCTIMEO) {
        serial_write("UHCI: TD CRC/timeout error\n");
        return 1;
    }

    if (status & UHCI_TD_BITSTUFF) {
        serial_write("UHCI: TD bitstuff error\n");
        return 1;
    }

    return 0;
}

int uhci_init(void) {
    uint16_t io_base;

    serial_write("UHCI: initialising...\n");

    if (uhci_find_controller(&io_base) != 0) {
        serial_write("UHCI: no controller found\n");
        return 1;
    }

    controller.io_base = io_base & 0xFFFC;

    serial_write("UHCI: controller found at I/O ");
    serial_write_hex((uint64_t)controller.io_base);
    serial_write("\n");

    if (uhci_reset() != 0) {
        return 1;
    }

    controller.initialised = 1;

    serial_write("UHCI: controller reset successfully\n");

    uhci_dump_ports();

    uhci_reset_port(1);

    for (uint32_t i = 0; i < UHCI_FRAME_COUNT; ++i) {
        uhci_frame_list[i] = UHCI_LINK_TERMINATE;
    }

    if (((uint32_t)uhci_frame_list & 0xFFF) != 0) {
        serial_write("UHCI: frame list is not 4K aligned\n");
        return 1;
    }

    uhci_write32(UHCI_REG_FRBASEADD, (uint32_t)uhci_frame_list);
    uhci_write16(UHCI_REG_FRNUM, 0);

    serial_write("UHCI: frame list = ");
    serial_write_hex((uint64_t)(uint32_t)uhci_frame_list);
    serial_write("\n");

    uhci_write32(UHCI_REG_FRBASEADD, (uint32_t)uhci_frame_list);

    // test
    uhci_setup_get_descriptor();
    uhci_frame_list[0] = (uint32_t)&uhci_setup_td;

    uhci_write16(UHCI_REG_FRNUM, 0);

    uhci_write16(UHCI_REG_USBCMD, UHCI_CMD_RS | UHCI_CMD_CONFIG | UHCI_CMD_MAXP);

    unsigned int timeout = 0;

    while (uhci_setup_td.status & UHCI_TD_ACTIVE) {
        if (++timeout >= 1000000) {
            serial_write("UHCI: transfer timed out\n");
            break;
        }
    }

    if (!uhci_td_failed(&uhci_setup_td) && !uhci_td_failed(&uhci_data_td) && !uhci_td_failed(&uhci_status_td)) {
        serial_write("UHCI: GET_DESCRIPTOR succeeded\n");

        for (uint32_t i = 0; i < 18; ++i) {
            serial_write_hex((uint64_t)uhci_device_descriptor[i]);
            serial_write(" ");
        }

        serial_write("\n");
    }

    serial_write("UHCI: setup status = ");
    serial_write_hex((uint64_t)uhci_setup_td.status);
    serial_write("\n");

    serial_write("UHCI: data status = ");
    serial_write_hex((uint64_t)uhci_data_td.status);
    serial_write("\n");

    serial_write("UHCI: status status = ");
    serial_write_hex((uint64_t)uhci_status_td.status);
    serial_write("\n");

    serial_write("UHCI: USBSTS = ");
    serial_write_hex((uint64_t)uhci_read16(UHCI_REG_USBSTS));
    serial_write("\n");

    return 0;
}

const uhci_controller_t *uhci_get_controller(void) {
    return &controller;
}