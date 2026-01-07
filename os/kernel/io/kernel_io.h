#pragma once
#include <stddef.h>
#include <stdint.h>

void put_char(char c, size_t *cursor_pos, uint8_t attr);
void put_string(const char *str, size_t *cursor_pos, uint8_t attr);
void clear_screen(size_t *cursor_pos);