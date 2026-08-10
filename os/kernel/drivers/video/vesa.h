#pragma once

#include <core/boot_info.h>
#include <lib/stdint.h>

void vesa_init(boot_info_t *info);

extern uint32_t vesa_width;
extern uint32_t vesa_height;
extern uint32_t vesa_pitch;
extern uint8_t vesa_bpp;
extern uint64_t vesa_framebuffer;

void vesa_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void vesa_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
void vesa_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void vesa_clear_screen(uint32_t color);

void vesa_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);
void vesa_draw_string(const char *str, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);
void vesa_scroll(uint32_t lines, uint32_t bg_color);
