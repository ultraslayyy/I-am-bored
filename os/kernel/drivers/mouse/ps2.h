#pragma once
#include <lib/stdint.h>

typedef enum {
    PS2_IM_NONE,     // standard (3-byte packets)
    PS2_IM_WHEEL,    // wheel (4-byte packets)
    PS2_IM_EXTRABTNS // 5 buttons + wheel
} intellimouse_level_t;

typedef struct {
    int x;
    int y;

    int dx;
    int dy;

    int left_b;
    int right_b;
    int middle_b;

    int wheel;

    int b4;
    int b5;

    int enabled;

    intellimouse_level_t type;
} mouse_state_t;

void mouse_init(void);
void mouse_handler(void);

mouse_state_t mouse_get_state(void);
void set_mouse_enabled(int enabled);