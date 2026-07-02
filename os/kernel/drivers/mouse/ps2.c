#include <arch.h>
#include <drivers/video/vesa.h>
#include "ps2.h"

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_COMMAND 0x64

static volatile mouse_state_t mouse;

static uint8_t packet[3];
static uint8_t cycle = 0;

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

void mouse_handler(void) {
    uint8_t data = inb(PS2_DATA);

    switch (cycle) {
        case 0:
            if (!(data & 0x08)) return;
            packet[0] = data;
            cycle++;
            break;
        case 1:
            packet[1] = data;
            cycle++;
            break;
        case 2:
            packet[2] = data;
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

            break;
    }
}

mouse_state_t mouse_get_state(void) {
    return mouse;
}

void set_mouse_enabled(int enabled) {
    mouse.enabled = enabled;
}

void mouse_init(void) {
    
    ps2_wait_input();
    outb(PS2_COMMAND, 0xA8);
    
    ps2_wait_input();
    outb(PS2_COMMAND, 0x20);
    ps2_wait_output();
    
    uint8_t status = inb(PS2_DATA);
    status |= 0x02; // Enable IRQ12 (PS/2)
    status |= 0x01; // Enable IRQ1 (keyboard)
    status &= ~0x20;
    
    ps2_wait_input();
    outb(PS2_COMMAND, 0x60);
    ps2_wait_input();
    outb(PS2_DATA, status);
    
    // Default settings
    mouse_write(0xF6);
    mouse_read();
    
    // Enable streaming
    mouse_write(0xF4);
    mouse_read(); // ACK
    
    mouse.x = vesa_width / 2;
    mouse.y = vesa_height / 2;

    mouse.enabled = 0;
}