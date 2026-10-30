#include <arch.h>
#include <drivers/pci/pci.h>
#include <drivers/serial/serial.h>
#include <lib/stdint.h>
#include <timer/timer.h>
#include "uhci.h"
#include "usb.h"

static uhci_controller_t controller = {0};
static uint32_t uhci_frame_list[UHCI_FRAME_COUNT] __attribute__((aligned(4096)));

static uhci_td_t uhci_transfer_tds[UHCI_MAX_TDS] __attribute__((aligned(4096)));
static uint8_t uhci_transfer_setup[8] __attribute__((aligned(16)));

static usb_device_t usb_devices[2];
static uint32_t usb_device_count = 0;
static uint8_t usb_descriptor_buffer[18] __attribute__((aligned(16)));

static uint32_t uhci_phys(void *ptr) {
    return (uint32_t)ptr;
}

static void usb_dump_device(const usb_device_t *device) {
    const usb_device_descriptor_t *desc = &device->descriptor;

    serial_write("\nUSB Device:\n");

    serial_write("  Port: ");
    serial_write_dec(device->port);
    serial_write("\n");

    serial_write("  Address: ");
    serial_write_dec(device->address);
    serial_write("\n");

    serial_write("  Speed: ");
    serial_write(device->low_speed ? "Low-speed" : "Full-speed");
    serial_write("\n");

    serial_write("  USB Version: ");
    serial_write_hex(desc->usb_version);
    serial_write("\n");

    serial_write("  Device Class: ");
    serial_write_hex(desc->device_class);
    serial_write("\n");

    serial_write("  Vendor ID: ");
    serial_write_hex(desc->vendor_id);
    serial_write("\n");

    serial_write("  Product ID: ");
    serial_write_hex(desc->product_id);
    serial_write("\n");

    serial_write("  Device Version: ");
    serial_write_hex(desc->device_version);
    serial_write("\n");

    serial_write("  EP0 Max Packet: ");
    serial_write_dec(desc->max_packet_size);
    serial_write("\n");

    serial_write("  Configurations: ");
    serial_write_dec(desc->num_configurations);
    serial_write("\n");
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

    delay_ms(50);

    value  = uhci_read16(reg);
    value &= ~UHCI_PORT_RESET;
    uhci_write16(reg, value);

    delay_ms(10);

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

static void uhci_init_td(uhci_td_t *td, uint32_t link, uint32_t token, uint32_t buffer, int low_speed) {
    td->link = link;
    td->status = UHCI_TD_ACTIVE | UHCI_TD_C_ERR_MASK | (low_speed ? UHCI_TD_LS : 0);
    td->token = token;
    td->buffer = buffer;
}

static int uhci_control_transfer(uint8_t device, uint8_t endpoint, uint8_t max_packet, int low_speed, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, void *buffer, uint16_t length) {
    uint32_t td_count = 0;
    uint32_t i;

    uint8_t data_pid;
    uint8_t status_pid;
    uint8_t toggle = 1;

    uint8_t *data = (uint8_t *)buffer;

    if (max_packet == 0 || max_packet > 64) {
        return 1;
    }

    if (length && !buffer) {
        return 1;
    }

    if (length > (UHCI_MAX_TDS - 2) * max_packet) {
        return 1;
    }

    uhci_transfer_setup[0] = request_type;
    uhci_transfer_setup[1] = request;

    uhci_transfer_setup[2] = (uint8_t)value;
    uhci_transfer_setup[3] = (uint8_t)(value >> 8);

    uhci_transfer_setup[4] = (uint8_t)index;
    uhci_transfer_setup[5] = (uint8_t)(index >> 8);
    
    uhci_transfer_setup[6] = (uint8_t)length;
    uhci_transfer_setup[7] = (uint8_t)(length >> 8);
    
    data_pid   = (request_type & 0x80) ? UHCI_PID_IN : UHCI_PID_OUT;
    status_pid = (data_pid == UHCI_PID_IN) ? UHCI_PID_OUT : UHCI_PID_IN;

    uint32_t setup_index = td_count++;
    uint32_t remaining   = length;
    uint32_t offset      = 0;

    while (remaining > 0) {
        uint16_t packet_size = remaining > max_packet ? max_packet : remaining;
        uint32_t data_index  = td_count++;
        
        uint32_t token = uhci_make_token(data_pid, device, endpoint, toggle, packet_size);
        uhci_init_td(&uhci_transfer_tds[data_index], UHCI_LINK_TERMINATE, token, uhci_phys(data + offset), low_speed);

        remaining -= packet_size;
        offset += packet_size;

        toggle ^= 1;
    }

    uint32_t status_index = td_count++;

    uhci_init_td(&uhci_transfer_tds[status_index], UHCI_LINK_TERMINATE, uhci_make_token(status_pid, device, endpoint, 1, 0), 0, low_speed);
    uhci_init_td(&uhci_transfer_tds[setup_index], UHCI_LINK_TERMINATE, uhci_make_token(UHCI_PID_SETUP, device, endpoint, 0, 8), uhci_phys(uhci_transfer_setup), low_speed);

    for (i = 0; i + 1 < td_count; ++i) {
        uhci_transfer_tds[i].link = uhci_phys(&uhci_transfer_tds[i+1]) | UHCI_LINK_DEPTH;
    }

    uhci_transfer_tds[td_count - 1].link = UHCI_LINK_TERMINATE;

    for (i = 0; i < UHCI_FRAME_COUNT; ++i) {
        uhci_frame_list[i] = uhci_phys(&uhci_transfer_tds[0]);
    }

    uhci_write16(UHCI_REG_FRNUM, 0);
    uhci_write16(UHCI_REG_USBSTS, UHCI_STS_USBINT | UHCI_STS_ERROR | UHCI_STS_RD | UHCI_STS_HSE | UHCI_STS_HCPE);

    __asm__ volatile("" ::: "memory");

    uhci_write16(UHCI_REG_USBCMD, UHCI_CMD_RS | UHCI_CMD_CONFIG | UHCI_CMD_MAXP);

    int result = 1;
    unsigned int timeout;

    for (timeout = 0; timeout < 500; ++timeout) {
        int failed = 0;

        for (i = 0; i < td_count; ++i) {
            uint32_t status = uhci_transfer_tds[i].status;

            if (status & (UHCI_TD_STALLED | UHCI_TD_DBUFFER | UHCI_TD_BABBLE | UHCI_TD_CRCTIMEO | UHCI_TD_BITSTUFF)) {
                failed = 1;
                break;
            }
        }

        if (failed) {
            break;
        }

        /* if (!(uhci_transfer_tds[status_index].status & UHCI_TD_ACTIVE)) {
            result = 0;
            break;
        } */
        //
        uint32_t status = uhci_transfer_tds[status_index].status;

        if ((timeout % 100) == 0) {
            serial_write("UHCI DEBUG: status TD = ");
            serial_write_hex(status);
            serial_write("\n");

            serial_write("UHCI DEBUG: active = ");
            serial_write_dec(!!(status & UHCI_TD_ACTIVE));
            serial_write("\n");
        }

        if (!(status & UHCI_TD_ACTIVE)) {
            serial_write("UHCI DEBUG: status TD completed!\n");
            result = 0;
            break;
        }
        //

        delay_ms(1);
    }

    uhci_write16(UHCI_REG_USBCMD, uhci_read16(UHCI_REG_USBCMD) & ~UHCI_CMD_RS);

    for (timeout = 0; timeout < 100000; ++timeout) {
        if (uhci_read16(UHCI_REG_USBSTS) & UHCI_STS_HCH) {
            break;
        }
    }

    __asm__ volatile ("" ::: "memory");

    for (i = 0; i < UHCI_FRAME_COUNT; ++i) {
        uhci_frame_list[i] = UHCI_LINK_TERMINATE;
    }

    if (timeout >= 100000) {
        serial_write("UHCI: failed to stop controller\n");
        return 1;
    }

    if (result != 0) {
        serial_write("UHCI: control transfer failed or timed out\n");

        for (i = 0; i < td_count; ++i) {
            serial_write("  TD ");
            serial_write_dec(i);
            serial_write(": ");
            serial_write_hex(uhci_transfer_tds[i].status);
            serial_write("\n");
        }

        return 1;
    }

    return 0;
}

static int usb_set_address(uint8_t old_address, uint8_t new_address, uint8_t max_packet, int low_speed) {
    if (uhci_control_transfer(old_address, 0, max_packet, low_speed, 0x00, USB_REQ_SET_ADDRESS, new_address, 0, 0, 0) != 0) {
        return 1;
    }

    delay_ms(2);
    return 0;
}

static int usb_enumerate_port(unsigned int port, uint8_t address) {
    uint16_t port_status;
    uint8_t low_speed;
    uint8_t max_packet;

    usb_device_t *device;

    serial_write("\nUSB: enumerating port ");
    serial_write_dec(port);
    serial_write("\n");

    port_status = uhci_get_port(port);

    if (!(port_status & UHCI_PORT_CCS)) {
        return 1;
    }

    low_speed = !!(port_status & UHCI_PORT_LSDA);

    if (uhci_reset_port(port) != 0) {
        return 1;
    }

    max_packet = 8;

    for (uint32_t i = 0; i < 8; ++i) {
        usb_descriptor_buffer[i] = 0;
    }

    if (uhci_control_transfer(0, 0, max_packet, low_speed, 0x80, USB_REQ_GET_DESCRIPTOR, (USB_DESC_DEVICE << 8), 0, usb_descriptor_buffer, 8) != 0) {
        serial_write("USB: failed to retrieve initial descriptor\n");
        return 1;
    }

    if (usb_descriptor_buffer[0] != 18 || usb_descriptor_buffer[1] != USB_DESC_DEVICE) {
        serial_write("USB: invalid device descriptor\n");
        return 1;
    }

    max_packet = usb_descriptor_buffer[7];

    if (low_speed) {
        if (max_packet != 8) {
            serial_write("USB: invalid low-speed EP0 packet size\n");
            return 1;
        }
    } else {
        if (max_packet != 8 && max_packet != 16 && max_packet != 32 && max_packet != 64) {
            serial_write("USB: invalid EP0 packet size\n");
            return 1;
        }
    }

    if (usb_set_address(0, address, max_packet, low_speed) != 0) {
        serial_write("USB: SET_ADDRESS failed\n");
        return 1;
    }

    for (uint32_t i = 0; i < 18; ++i) {
        usb_descriptor_buffer[i] = 0;
    }

    if (uhci_control_transfer(address, 0, max_packet, low_speed, 0x80, USB_REQ_GET_DESCRIPTOR, (USB_DESC_DEVICE << 8), 0, usb_descriptor_buffer, 18) != 0) {
        serial_write("USB: failed to retrieve full descriptor\n");
        return 1;
    }

    device = &usb_devices[usb_device_count];

    device->address   = address;
    device->port      = port;
    device->low_speed = low_speed;

    usb_parse_device_descriptor(usb_descriptor_buffer, &device->descriptor);

    usb_device_count++;

    serial_write("USB: device enumerated successfully\n");

    usb_dump_device(device);

    return 0;
}

static void usb_enumerate_devices(void) {
    uint8_t next_address = 1;

    usb_device_count = 0;

    for (unsigned int port = 1; port <= 2; ++port) {
        if (usb_device_count >= 2) {
            break;
        }

        if (usb_enumerate_port(port, next_address) == 0) {
            next_address++;
        }
    }

    serial_write("\nUSB: enumeration complete\n");

    serial_write("USB: devices found: ");
    serial_write_dec(usb_device_count);
    serial_write("\n");
}

int uhci_init(void) {
    uint16_t io_base;

    serial_write("UHCI: initialising...\n");

    if (uhci_find_controller(&io_base) != 0) {
        serial_write("UHCI: no controller found\n");
        return 1;
    }

    serial_write("TD size = ");
    serial_write_dec(sizeof(uhci_td_t));
    serial_write("\n");

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

    for (uint32_t i = 0; i < UHCI_FRAME_COUNT; ++i) {
        uhci_frame_list[i] = UHCI_LINK_TERMINATE;
    }

    controller.frame_list = uhci_frame_list;

    uhci_write32(UHCI_REG_FRBASEADD, uhci_phys(uhci_frame_list));
    uhci_write16(UHCI_REG_FRNUM, 0);

    serial_write("UHCI: frame list = ");
    serial_write_hex((uint64_t)uhci_phys(uhci_frame_list));
    serial_write("\n");

    usb_enumerate_devices();

    return 0;
}

const uhci_controller_t *uhci_get_controller(void) {
    return &controller;
}