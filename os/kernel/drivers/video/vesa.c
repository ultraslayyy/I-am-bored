#include <arch.h>
#include <lib/string.h>
#include "font.h"
#include "vesa.h"

uint32_t vesa_width = 0;
uint32_t vesa_height = 0;
uint32_t vesa_pitch = 0;
uint8_t vesa_bpp = 0;
uint64_t vesa_framebuffer = 0;

void vesa_init(boot_info_t *info) {
    vesa_width = info->framebuffer_width;
    vesa_height = info->framebuffer_height;
    vesa_pitch = info->framebuffer_pitch;
    vesa_bpp = info->framebuffer_bpp;
    vesa_framebuffer = info->framebuffer_addr;

    uint32_t fb_size = vesa_height * vesa_pitch;
    uint32_t fb_pages = (fb_size + 0xFFF) / PAGE_SIZE;
    
    extern void map_page(uint32_t virt, uint32_t phys, uint32_t flags);
    
    for (uint32_t i = 0; i < fb_pages; ++i) {
        uint32_t base = vesa_framebuffer & 0xFFFFF000;
        map_page(base + i * PAGE_SIZE, base + i * PAGE_SIZE, PAGE_PRESENT | PAGE_RW);
    }
}

void vesa_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= vesa_width || y >= vesa_height) return;

    if (vesa_bpp == 32) {
        uint32_t *fb = (uint32_t *)(vesa_framebuffer + y * vesa_pitch + x * 4);
        *fb = color;
    } else if (vesa_bpp == 24) {
        uint8_t *fb = (uint8_t *)(vesa_framebuffer + y * vesa_pitch + x * 3);
        fb[0] = color & 0xFF;           // Blue
        fb[1] = (color >> 8) & 0xFF;    // Green
        fb[2] = (color >> 16) & 0xFF;   // Red
    }
}

void vesa_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t r = y; r < y + height; ++r) {
        for (uint32_t c = x; c < x + width; ++c) {
            vesa_draw_pixel(c, r, color);
        }
    }
}

void vesa_clear_screen(uint32_t color) {
    vesa_draw_rect(0, 0, vesa_width, vesa_height, color);
}

void vesa_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    if (x >= vesa_width || y >= vesa_height) return;

    for (uint32_t row = 0; row < FONT_HEIGHT; ++row) {
        uint16_t line = font[(uint8_t)c][row];
        for (uint32_t col = 0; col < FONT_WIDTH; ++col) {
            uint32_t px = x + col;
            uint32_t py = y + row;

            if (px >= vesa_width || py >= vesa_height) continue;

            if ((line >> (15 - col)) & 1) {
                vesa_draw_pixel(px, py, fg_color);
            } else {
                vesa_draw_pixel(px, py, bg_color);
            }
        }
    }
}

void vesa_draw_string(const char *str, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    uint32_t cx = x;
    uint32_t cy = y;

    for (size_t i = 0; str[i]; ++i) {
        if (str[i] == '\n') {
            cx = x;
            cy += FONT_HEIGHT;
            continue;
        }

        vesa_draw_char(str[i], cx, cy, fg_color, bg_color);
        cx += FONT_WIDTH;
        if (cx + FONT_WIDTH >= vesa_width) {
            cx = x;
            cy += FONT_HEIGHT;
        }
    }
}

void vesa_scroll(uint32_t lines, uint32_t bg_color) {
    if (lines * FONT_HEIGHT >= vesa_height) {
        vesa_clear_screen(bg_color);
        return;
    }

    uint32_t bytes_to_scroll = lines * FONT_HEIGHT * vesa_pitch;
    uint32_t total_bytes = vesa_height * vesa_pitch;

    memmove((void *)vesa_framebuffer, (void *)(vesa_framebuffer + bytes_to_scroll), total_bytes - bytes_to_scroll);

    vesa_draw_rect(0, vesa_height - (lines * FONT_HEIGHT), vesa_width, lines * FONT_HEIGHT, bg_color);
}
