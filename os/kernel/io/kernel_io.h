#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>
#include <drivers/video/vga.h>

#define DEFAULT_ATTR 0x07
#define BUFFER_HEIGHT 1024
#define ROW_IDX(r) ((r) % BUFFER_HEIGHT)

typedef struct {
    char chars[VGA_WIDTH];
    uint8_t attrs[VGA_WIDTH];
} line_t;

extern line_t text_buffer[BUFFER_HEIGHT];
extern size_t total_lines;
extern size_t cursor_row;
extern size_t cursor_col;
extern size_t viewport_top;

void put_char(char c, uint8_t attr);
void put_string(const char *str, uint8_t attr);
void clear_screen(void);
void update_screen(void);
void scroll_viewport(void);
void scroll_up(void);
void scroll_down(void);