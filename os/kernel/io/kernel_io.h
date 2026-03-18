#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>
#include <drivers/video/vesa.h>

#define DEFAULT_ATTR 0x07

extern uint32_t cursor_x;
extern uint32_t cursor_y;
extern uint32_t text_fg;
extern uint32_t text_bg;

void put_char(char c, uint8_t attr);
void put_string(const char *str, uint8_t attr);
void clear_screen(void);
void update_screen(void);
void scroll_up(void);
void scroll_down(void);