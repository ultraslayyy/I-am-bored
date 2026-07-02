#pragma once
#include <lib/stdint.h>

typedef struct {
    int x;
    int y;

    int dx;
    int dy;

    int left_b;
    int right_b;
    int middle_b;

    int enabled;
} mouse_state_t;

void mouse_init(void);
void mouse_handler(void);

mouse_state_t mouse_get_state(void);
void set_mouse_enabled(int enabled);