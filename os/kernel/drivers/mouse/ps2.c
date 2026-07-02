#include <arch.h>
#include <drivers/video/vesa.h>
#include "ps2.h"

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_COMMAND 0x64

static volatile mouse_state_t mouse;

static uint8_t packet[4];
static uint8_t cycle = 0;
static uint8_t packet_size = 3;

static void ps2_wait_input() {
    while (inb(PS2_STATUS) & 0x02);
}

static void ps2_wait_output() {
    while (!(inb(PS2_STATUS) & 0x01));
}

static void mouse_write(uint8_t data) {
    ps2_wait_input();
    outb(PS2_COMMAND, 0xD4);

    ps2_wait_input();
    outb(PS2_DATA, data);
}

static uint8_t mouse_read() {
    ps2_wait_output();
    return inb(PS2_DATA);
}

static uint8_t mouse_expect_ack(void) {
    return mouse_read() == 0xFA;
}

void mouse_handler(void) {
    if (!mouse.enabled) return;

    uint8_t data = inb(PS2_DATA);

    // desync check
    if (cycle == 0) {
        if (!(data & 0x08)) return;
    } else {
        if ((data & 0x08) && cycle != 0) {
            cycle = 0;
            return;
        }
    }

    packet[cycle] = data;
    cycle++;

    if (cycle < packet_size) return;

    cycle = 0;

    mouse.dx = (int8_t)packet[1];
    mouse.dy = (int8_t)packet[2];

    mouse.x += mouse.dx;
    mouse.y -= mouse.dy; // Invert Y

    if (mouse.x < 0) {
        mouse.x = 0;
    }
    if (mouse.y < 0) {
        mouse.y = 0;
    }
    if (mouse.x > vesa_width) {
        mouse.x = vesa_width;
    }
    if (mouse.y > vesa_height) {
        mouse.y = vesa_height;
    }

    mouse.left_b   = packet[0] & 0x1;
    mouse.right_b  = packet[0] & 0x2;
    mouse.middle_b = packet[0] & 0x4;

    if (mouse.type == PS2_IM_EXTRABTNS) {
        mouse.b4 = (packet[3] & 0x10) ? 1 : 0;
        mouse.b5 = (packet[3] & 0x20) ? 1 : 0;
        mouse.wheel = (int8_t)(packet[3] << 4) >> 4;
    } else if (mouse.type == PS2_IM_WHEEL) {
        mouse.wheel = (int8_t)packet[3];
    } else {
        mouse.wheel = 0;
    }
}

mouse_state_t mouse_get_state(void) {
    return mouse;
}

void set_mouse_enabled(int enabled) {
    mouse.enabled = enabled;
}

static uint8_t mouse_probe_intellimouse(void) {
    mouse_write(0xF3);
    if (!mouse_expect_ack()) return 0x00;

    mouse_write(200);
    if (!mouse_expect_ack()) return 0x00;

    mouse_write(0xF3);
    if (!mouse_expect_ack()) return 0x00;

    mouse_write(100);
    if (!mouse_expect_ack()) return 0x00;

    mouse_write(0xF3);
    if (!mouse_expect_ack()) return 0x00;

    mouse_write(80);
    if (!mouse_expect_ack()) return 0x00;

    mouse_write(0xF2);
    return mouse_read();
}

void mouse_init(void) {
    
    ps2_wait_input();
    outb(PS2_COMMAND, 0xA8);
    
    ps2_wait_input();
    outb(PS2_COMMAND, 0x20);
    ps2_wait_output();
    
    uint8_t status = inb(PS2_DATA);
    status |= 0x02;  // Enable IRQ12 (PS/2)
    status |= 0x01;  // Enable IRQ1 (keyboard)
    status &= ~0x20; // enable mouse clock
    
    ps2_wait_input();
    outb(PS2_COMMAND, 0x60);

    ps2_wait_input();
    outb(PS2_DATA, status);

    // Reset scaling
    mouse_write(0xE6);
    mouse_expect_ack();

    // Check for intellimouse support
    uint8_t id = mouse_probe_intellimouse();

    if (id == 0x00) {
        mouse.type = PS2_IM_NONE;
        packet_size = 3;
    } else if (id == 0x03) {
        mouse.type = PS2_IM_WHEEL;
        packet_size = 4;
    } else if (id == 0x04) {
        mouse.type = PS2_IM_EXTRABTNS;
        packet_size = 4;
    }

    // Default settings
    mouse_write(0xF6);
    mouse_expect_ack();
    
    // Enable streaming
    mouse_write(0xF4);
    mouse_expect_ack();

    mouse.x = vesa_width / 2;
    mouse.y = vesa_height / 2;

    mouse.enabled = 0;
}