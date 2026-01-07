#include <drivers/video/vga.h>
#include "kernel_io.h"

line_t text_buffer[BUFFER_HEIGHT];
size_t buffer_lines = 0;
size_t viewport_top = 0;
size_t cursor_col = 0; 
size_t cursor_row = 0;

static void scroll_if_needed(size_t *cursor_pos) {
    size_t row = *cursor_pos / VGA_WIDTH;
    if (row < VGA_HEIGHT) return;

    char *vidmem = VGA_MEMORY;
    size_t lines_to_scroll = row - VGA_HEIGHT + 1;

    for (size_t r = 0; r < VGA_HEIGHT - lines_to_scroll; ++r) {
        for (size_t c = 0; c < VGA_WIDTH; ++c) {
            vidmem[(r * VGA_WIDTH + c) * 2] =
                vidmem[((r + lines_to_scroll) * VGA_WIDTH + c) * 2];
            vidmem[(r * VGA_WIDTH + c) * 2 + 1] =
                vidmem[((r + lines_to_scroll) * VGA_WIDTH + c) * 2 + 1];
        }
    }

    for (size_t r = VGA_HEIGHT - lines_to_scroll; r < VGA_HEIGHT; ++r) {
        for (size_t c = 0; c < VGA_WIDTH; ++c) {
            vidmem[(r * VGA_WIDTH + c) * 2] = ' ';
            vidmem[(r * VGA_WIDTH + c) * 2 + 1] = DEFAULT_ATTR;
        }
    }

    *cursor_pos -= lines_to_scroll * VGA_WIDTH;
}

void put_char(char c, uint8_t attr) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= BUFFER_HEIGHT) cursor_row = BUFFER_HEIGHT - 1;
        if (cursor_row >= buffer_lines) buffer_lines = cursor_row + 1;
        scroll_viewport();
        update_screen();
        return;
    }

    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= BUFFER_HEIGHT) cursor_row = BUFFER_HEIGHT - 1;
        if (cursor_row >= buffer_lines) buffer_lines = cursor_row + 1;
        scroll_viewport();
        update_screen();
    }

    text_buffer[cursor_row].chars[cursor_col] = c;
    text_buffer[cursor_row].attrs[cursor_col] = attr;
    cursor_col++;

    if (cursor_row >= buffer_lines) buffer_lines = cursor_row + 1;

    update_screen();
}

void put_string(const char *str, uint8_t attr) {
    for (size_t i = 0; str[i]; ++i)
        put_char(str[i], attr);
}

void clear_screen(void) {
    for (size_t i = 0; i < VGA_HEIGHT; i++) {
        for (size_t j = 0; j < VGA_WIDTH; j++) {
            text_buffer[i].chars[j] = ' ';
            text_buffer[i].attrs[j] = DEFAULT_ATTR;
        }
    }
    buffer_lines = 1;
    cursor_row = 0;
    cursor_col = 0;
    viewport_top = 0;
    update_screen();
}

void update_screen(void) {
    char *vidmem = VGA_MEMORY;

    for (size_t row = 0; row < VGA_HEIGHT; ++row) {
        size_t buf_row = viewport_top + row;
        for (size_t col = 0; col < VGA_WIDTH; ++col) {
            if (buf_row < buffer_lines) {
                vidmem[(row * VGA_WIDTH + col) * 2] = text_buffer[buf_row].chars[col];
                vidmem[(row * VGA_WIDTH + col) * 2 + 1] = text_buffer[buf_row].attrs[col];
            } else {
                vidmem[(row * VGA_WIDTH + col) * 2] = ' ';
                vidmem[(row * VGA_WIDTH + col) * 2 + 1] = 0x07;
            }
        }
    }
}

void scroll_viewport(void) {
    if (cursor_row >= viewport_top + VGA_HEIGHT) {
        viewport_top = cursor_row - VGA_HEIGHT + 1;
    }
}

void scroll_up(void) {
    if (viewport_top > 0) viewport_top--;
    update_screen();
}

void scroll_down(void) {
    if (viewport_top + VGA_HEIGHT < buffer_lines) viewport_top++;
    update_screen();
}
