#include <io/kernel_io.h>
#include "ehci.h"
#include "uhci.h"
#include "usb.h"

void usb_init(void) {
    /*
    if (!ehci_init()) {
        put_string("EHCI not found\n", DEFAULT_ATTR);
        return;
    }

    put_string("EHCI initialised\n", DEFAULT_ATTR);

    ehci_probe_ports();
    */

    if (!uhci_init()) {
        put_string("UHCI not found\n", DEFAULT_ATTR);
        return;
    }

    put_string("UHCI initialised\n", DEFAULT_ATTR);
}

uint16_t usb_read16(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

void usb_parse_device_descriptor(const uint8_t *data, usb_device_descriptor_t *desc) {
    desc->length             = data[0];
    desc->descriptor_type    = data[1];

    desc->usb_version        = usb_read16(&data[2]);

    desc->device_class       = data[4];
    desc->device_subclass    = data[5];
    desc->device_protocol    = data[6];

    desc->max_packet_size    = data[7];

    desc->vendor_id          = usb_read16(&data[8]);
    desc->product_id         = usb_read16(&data[10]);
    desc->device_version     = usb_read16(&data[12]);

    desc->manufacturer_index = data[14];
    desc->product_index      = data[15];
    desc->serial_index       = data[16];

    desc->num_configurations = data[17];
}