#pragma once
#include <core/boot_info.h>
#include <lib/stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t  bpp;

    uint64_t address;
} framebuffer_t;

/* later when virtio-gpu is added
typedef struct {
    int (*init)(void *config);

    void (*draw_pixel)(uint32_t x, uint32_t y, uint32_t color);
    ...
} framebuffer_backend_t;
*/

extern framebuffer_t framebuffer;

int framebuffer_init(boot_info_t *info);
void framebuffer_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void framebuffer_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
void framebuffer_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void framebuffer_clear(uint32_t color);
void framebuffer_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);
void framebuffer_draw_string(const char *str, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);
void framebuffer_scroll(uint32_t lines, uint32_t bg_color);