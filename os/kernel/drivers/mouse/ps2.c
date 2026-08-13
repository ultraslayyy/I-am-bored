#include <arch.h>
#include <drivers/video/vesa.h>
#include "ps2.h"

#define PS2_COMMAND 0x64

#define PS2_TIMEOUT 100000

static volatile mouse_state_t mouse;

static uint8_t packet[4];
static uint8_t cycle = 0;
static uint8_t packet_size = 3;

static int ps2_wait_input() {
    for (int i = 0; i < PS2_TIMEOUT; ++i) {
        if (!(inb(PS2_STATUS) & 0x02)) return 1;
    }
    return 0;
}

static int ps2_wait_output() {
    for (int i = 0; i < PS2_TIMEOUT; ++i) {
        if (inb(PS2_STATUS) & 0x01) return 1;
    }
    return 0;
}

static int mouse_write(uint8_t data) {
    if (!ps2_wait_input()) return 0;

    outb(PS2_COMMAND, 0xD4);

    if (!ps2_wait_input()) return 0;

    outb(PS2_DATA, data);
    return 1;
}

static int mouse_read(uint8_t *data) {
    if (!ps2_wait_output()) return 0;
    *data = inb(PS2_DATA);
    return 1;
}

static int mouse_expect_ack(void) {
    uint8_t b;
    return mouse_read(&b) && b == 0xFA;
}

static int mouse_reset(uint8_t *id) {
    uint8_t data;

    if (!mouse_write(0xFF) || !mouse_expect_ack()) return 0;

    if (!mouse_read(&data) || data != 0xAA) return 0;

    if (!mouse_read(id)) return 0;

    return 1;
}

void mouse_handler(uint8_t data) {
    if (!mouse.enabled) return;

    // desync check
    if (cycle == 0 && !(data & 0x08)) return;

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
    if (mouse.x > vesa_width - 1) {
        mouse.x = vesa_width - 1;
    }
    if (mouse.y > vesa_height - 1) {
        mouse.y = vesa_height - 1;
    }

    mouse.left_b   = !!(packet[0] & 0x1);
    mouse.right_b  = !!(packet[0] & 0x2);
    mouse.middle_b = !!(packet[0] & 0x4);

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
    disable_interrupts();
    mouse_state_t m = mouse;
    enable_interrupts();
    return m;
}

int mouse_set_enabled(int enabled) {
    if (mouse.exists) {
        mouse.enabled = enabled;
        return 1;
    } else {
        return 0;
    }
}

static uint8_t mouse_set_rate(uint8_t rate) {
    if (!mouse_write(0xF3))  return 0x00;
    if (!mouse_expect_ack()) return 0x00;
    
    if (!mouse_write(rate))  return 0x00;
    if (!mouse_expect_ack()) return 0x00;

    return 0x01;
}

static uint8_t mouse_get_id(uint8_t *id) {
    if (!mouse_write(0xF2))  return 0xFF;
    if (!mouse_expect_ack()) return 0xFF;
    if (!mouse_read(id))     return 0xFF;
    return 0x00;
}

static uint8_t mouse_probe_intellimouse(int im2) {
    if (!mouse_set_rate(200))             return 0xFF;
    if (!mouse_set_rate(im2 ? 200 : 100)) return 0xFF;
    if (!mouse_set_rate(80))              return 0xFF;

    uint8_t id = 0;
    if (!mouse_get_id(&id)) {
        return 0xFF;
    }

    return id;
}

// Check Intellimouse Extension support
static uint8_t mouse_check_ime(void) {
    uint8_t id = mouse_probe_intellimouse(0);
    if (id == 0xFF) return 0xFF;

    if (id == 0x03) {
        id = mouse_probe_intellimouse(1);
        if (id == 0xFF) return 0xFF;
    }

    return id;
}

int mouse_init(void) {
    mouse.enabled = 0;

    if (!ps2_wait_input())  goto fail;

    outb(PS2_COMMAND, 0xA8);
    
    if (!ps2_wait_input())  goto fail;
    outb(PS2_COMMAND, 0x20);
    if (!ps2_wait_output()) goto fail;
    
    uint8_t status = inb(PS2_DATA);
    status |= 0x02;  // Enable IRQ12 (PS/2)
    status |= 0x01;  // Enable IRQ1 (keyboard)
    status &= ~0x20; // enable mouse clock
    
    if (!ps2_wait_input()) goto fail;
    outb(PS2_COMMAND, 0x60);

    if (!ps2_wait_input()) goto fail;
    outb(PS2_DATA, status);

    uint8_t id;

    if (!mouse_reset(&id)) goto fail;

    // Reset scaling
    if (!mouse_write(0xE6) || !mouse_expect_ack()) goto fail;

    // Check for intellimouse support
    id = mouse_check_ime();

    if (id == 0x00) {
        mouse.type = PS2_IM_NONE;
        packet_size = 3;
    } else if (id == 0x03) {
        mouse.type = PS2_IM_WHEEL;
        packet_size = 4;
    } else if (id == 0x04) {
        mouse.type = PS2_IM_EXTRABTNS;
        packet_size = 4;
    } else {
        goto fail;
    }

    // Default settings
    if (!mouse_write(0xF6) || !mouse_expect_ack()) goto fail;
    
    // Enable streaming
    if (!mouse_write(0xF4) || !mouse_expect_ack()) goto fail;

    mouse.x = vesa_width  / 2;
    mouse.y = vesa_height / 2;

    mouse.exists = 1;
    return 1;

fail:
    mouse.enabled = 0;
    mouse.exists  = 0;
    return 0;
}