#pragma once
#include <lib/stdint.h>

#define PS2_DATA   0x60
#define PS2_STATUS 0x64

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
    int exists;

    intellimouse_level_t type;
} mouse_state_t;

int mouse_init(void);
void mouse_handler(uint8_t data);

mouse_state_t mouse_get_state(void);
int mouse_set_enabled(int enabled);