#pragma once
#include <lib/stdint.h>

typedef struct {
    uint8_t scancode;
    uint8_t pressed; // 1 = pressed, 0 = released
} key_event_t;

#define KEY_QUEUE_SIZE 64

extern uint8_t shift_pressed;

int keyboard_pop_event(key_event_t *event);

char keycode_to_char(uint8_t scancode, uint8_t shift_pressed);

void keyboard_timer_tick(void);