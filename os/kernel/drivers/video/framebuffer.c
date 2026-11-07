#include <drivers/serial/serial.h>
#include <lib/stddef.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include "bochs.h"
#include "font.h"
#include "framebuffer.h"
#include "vesa.h"

framebuffer_t framebuffer = {0};

int framebuffer_init(boot_info_t *info) {
    /*if (bochs_vbe_init() == 0) {
        framebuffer.width   = bochs_vbe_width;
        framebuffer.height  = bochs_vbe_height;
        framebuffer.pitch   = bochs_vbe_pitch;
        framebuffer.bpp     = bochs_vbe_bpp;
        framebuffer.address = bochs_vbe_framebuffer;

        serial_write("Framebuffer initialised with Bochs\n");

        return 0;
    }*/

    vesa_init(info);

    framebuffer.width   = vesa_width;
    framebuffer.height  = vesa_height;
    framebuffer.pitch   = vesa_pitch;
    framebuffer.bpp     = vesa_bpp;
    framebuffer.address = vesa_framebuffer;

    if (!framebuffer.address || !framebuffer.width || !framebuffer.height || !framebuffer.pitch) {
        return -1;
    }

    serial_write("Framebuffer initialised with standard VESA/VBE\n");

    return 0;
}

void framebuffer_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= framebuffer.width || y >= framebuffer.height) return;

    if (framebuffer.bpp == 32) {
        uint32_t *fb = (uint32_t *)(framebuffer.address + y * framebuffer.pitch + x * 4);
        *fb = color;
    } else if (framebuffer.bpp == 24) {
        uint8_t *fb = (uint8_t *)(framebuffer.address + y * framebuffer.pitch + x * 3);
        fb[0] = color & 0xFF;           // Blue
        fb[1] = (color >> 8) & 0xFF;    // Green
        fb[2] = (color >> 16) & 0xFF;   // Red
    }
}

void framebuffer_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int32_t dx = abs((int32_t)x2 - (int32_t)x1);
    int32_t sx = x1 < x2 ? 1 : -1;

    int32_t dy = abs((int32_t)y2 - (int32_t)y1);
    int32_t sy = y1 < y2 ? 1 : -1;

    int32_t err = dx + dy;

    int32_t x = x1;
    int32_t y = y1;

    for (;;) {
        framebuffer_draw_pixel(x, y, color);

        if (x == (int32_t)x2 && y == (int32_t)y2) {
            break;
        }

        int32_t e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

void framebuffer_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (x >= framebuffer.width || y >= framebuffer.height) {
        return;
    }

    if (width > framebuffer.width - x) {
        width = framebuffer.width - x;
    }

    if (height > framebuffer.height - y) {
        height = framebuffer.height - y;
    }

    for (uint32_t r = y; r < y + height; ++r) {
        for (uint32_t c = x; c < x + width; ++c) {
            framebuffer_draw_pixel(c, r, color);
        }
    }
}

void framebuffer_clear(uint32_t color) {
    framebuffer_draw_rect(0, 0, framebuffer.width, framebuffer.height, color);
}

void framebuffer_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    if (x >= framebuffer.width || y >= framebuffer.height) {
        return;
    }

    for (uint32_t row = 0; row < FONT_HEIGHT; ++row) {
        uint16_t line = font[(uint8_t)c][row];

        for (uint32_t col = 0; col < FONT_WIDTH; ++col) {
            uint32_t px = x + col;
            uint32_t py = y + row;

            if (px >= framebuffer.width ||
                py >= framebuffer.height) {
                continue;
            }

            if ((line >> (15 - col)) & 1) {
                framebuffer_draw_pixel(px, py, fg_color);
            } else {
                framebuffer_draw_pixel(px, py, bg_color);
            }
        }
    }
}

void framebuffer_draw_string(const char *str, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    uint32_t cx = x;
    uint32_t cy = y;

    for (size_t i = 0; str[i]; ++i) {
        if (str[i] == '\n') {
            cx = x;
            cy += FONT_HEIGHT;
            continue;
        }

        framebuffer_draw_char(str[i], cx, cy, fg_color, bg_color);

        cx += FONT_WIDTH;

        if (cx + FONT_WIDTH >= framebuffer.width) {
            cx = x;
            cy += FONT_HEIGHT;
        }
    }
}

void framebuffer_scroll(uint32_t lines, uint32_t bg_color) {
    if (lines * FONT_HEIGHT >= framebuffer.height) {
        framebuffer_clear(bg_color);
        return;
    }

    uint32_t bytes_to_scroll = lines * FONT_HEIGHT * framebuffer.pitch;
    uint32_t total_bytes     = framebuffer.height * framebuffer.pitch;

    memmove((void *)framebuffer.address, (void *)(framebuffer.address + bytes_to_scroll), total_bytes - bytes_to_scroll);

    framebuffer_draw_rect(0, framebuffer.height - (lines * FONT_HEIGHT), framebuffer.width, lines * FONT_HEIGHT, bg_color);
}