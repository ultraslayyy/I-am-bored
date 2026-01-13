#pragma once
#include <lib/stdint.h>

char keycode_to_char(uint8_t scancode, uint8_t shift_pressed);
void handle_key_release(uint8_t released, uint8_t *shift_pressed);
void keyboard_callback(void);