#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include <stdint.h>

typedef struct {
    uint8_t  memory[4096];
    uint8_t  V[16];
    int      I;
    int      pc;
    uint16_t stack[16];
    int      sp;
    int      delay_timer;
    int      sound_timer;
    uint8_t  display[2048]; // 64*32
} chip8_cpu;

#endif