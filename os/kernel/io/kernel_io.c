#include <drivers/video/vga.h>
#include "kernel_io.h"

void put_char(char c, size_t *cursor_pos, uint8_t attr) {
    if (c == '\n') {
        *cursor_pos = ((*cursor_pos / VGA_WIDTH) + 1) * VGA_WIDTH;
        if (*cursor_pos >= VGA_WIDTH * VGA_HEIGHT) *cursor_pos = 0;
        return;
    }

    char *vidmem = VGA_MEMORY;
    vidmem[*cursor_pos * 2] = c;
    vidmem[*cursor_pos * 2 + 1] = attr;
    (*cursor_pos)++;
    if (*cursor_pos >= VGA_WIDTH * VGA_HEIGHT) *cursor_pos = 0;
}

void put_string(const char *str, size_t *cursor_pos, uint8_t attr) {
    for (size_t i = 0; str[i]; ++i)
        put_char(str[i], cursor_pos, attr);
}

void clear_screen(size_t *cursor_pos) {
    char *vidmem = VGA_MEMORY;
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vidmem[i*2] = ' ';
        vidmem[i*2 + 1] = 0x07;
    }
    *cursor_pos = 0;
}