#pragma once
#include <lib/stdint.h>

#define USB_DESC_DEVICE        0x01
#define USB_DESC_CONFIGURATION 0x02
#define USB_DESC_STRING        0x03
#define USB_DESC_INTERFACE     0x04
#define USB_DESC_ENDPOINT      0x05
#define USB_DESC_HID           0x21

#define USB_CLASS_HID         0x03
#define USB_SUBCLASS_BOOT     0x01
#define USB_PROTOCOL_KEYBOARD 0x01

#define USB_ENDPOINT_IN             0x80
#define USB_ENDPOINT_TYPE_INTERRUPT 0x03

#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_ADDRESS    0x05

#define USB_MAX_DEVICES     127
#define USB_MAX_PACKET_SIZE 64

#define USB_CONFIG_BUFFER_SIZE 256

typedef struct {
    uint8_t length;
    uint8_t descriptor_type;

    uint16_t usb_version;

    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;

    uint8_t max_packet_size;

    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_version;

    uint8_t manufacturer_index;
    uint8_t product_index;
    uint8_t serial_index;

    uint8_t num_configurations;
} usb_device_descriptor_t;

typedef struct {
    uint8_t address;
    uint8_t port;
    uint8_t low_speed;

    usb_device_descriptor_t descriptor;
} usb_device_t;

void usb_init(void);
void usb_parse_device_descriptor(const uint8_t *data, usb_device_descriptor_t *desc);
uint16_t usb_read16(const uint8_t *data);