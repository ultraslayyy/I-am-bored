#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cpu.h"

uint8_t FONT[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void cpu_init(chip8_cpu *cpu) {
    cpu->pc = 0x200;
    memcpy(&cpu->memory[0], FONT, sizeof(FONT));

    srand(time(NULL));
}

void load_program(chip8_cpu *cpu, uint8_t *program, size_t program_size) {
    memcpy(&cpu->memory[0x200], program, program_size);
}

int execute_opcode(chip8_cpu *cpu, int opcode) {
    int X   = (opcode & 0x0F00) >> 8;
    int Y   = (opcode & 0x00F0) >> 4;
    int N   =  opcode & 0x000F;
    int NN  =  opcode & 0x00FF;
    int NNN =  opcode & 0x0FFF;

    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) {
                for (size_t i = 0; i < 4096; ++i) {
                    cpu->display[i] = 0;
                }
            } else if (opcode == 0x00EE) {
                if (cpu->sp == 0) {
                    fprintf(stderr, "Stack underflow");
                    return 1;
                }

                cpu->pc = cpu->stack[--cpu->sp];
            }
            break;
        case 0x1000:
            cpu->pc = NNN;
            break;
        case 0x2000:
            if (cpu->sp >= 16) {
                fprintf(stderr, "Stack overflow");
                return 2;
            }

            cpu->stack[cpu->sp++] = cpu->pc;
            cpu->pc = NNN;
            break;
        case 0x3000:
            if (cpu->V[X] == NN) {
                cpu->pc += 2;
            }
            break;
        case 0x4000:
            if (cpu->V[X] != NN) {
                cpu->pc += 2;
            }
            break;
        case 05000:
            if (N != 0) {
                fprintf(stderr, "Unknown opcode 0x%04X", opcode);
                return 3;
            }

            if (cpu->V[X] == cpu->V[Y]) {
                cpu->pc += 2;
            }
            break;
        case 0x6000:
            cpu->V[X] = NN;
            break;
        case 0x7000:
            cpu->V[X] = (cpu->V[X] + NN) & 0xFF;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0:
                    cpu->V[X] = cpu->V[Y];
                    break;
                case 0x1:
                    cpu->V[X] |= cpu->V[Y];
                    break;
                case 0x2:
                    cpu->V[X] &= cpu->V[Y];
                    break;
                case 0x3:
                    cpu->V[X] ^= cpu->V[Y];
                    break;
                case 0x4: {
                    int sum = cpu->V[X] + cpu->V[Y];
                    cpu->V[0xF] = sum > 0xFF ? 1 : 0;
                    cpu->V[X] = sum & 0xFF;
                    break;
                }
                case 0x5:
                    cpu->V[0xF] = cpu->V[X] >= cpu->V[Y] ? 1 : 0;
                    cpu->V[X] = (cpu->V[X] - cpu->V[Y]) & 0xFF;
                    break;
                case 0x6:
                    cpu->V[0xF] = cpu->V[X] & 1;
                    cpu->V[X] >>= 1;
                    break;
                case 0x7:
                    cpu->V[0xF] = cpu->V[Y] >= cpu->V[X] ? 1 : 0;
                    cpu->V[X] = (cpu->V[Y] - cpu->V[X]) & 0xFF;
                    break;
                case 0xE: 
                    cpu->V[0xF] = (cpu->V[X] >> 7) & 1;
                    cpu->V[X] = (cpu->V[X] << 1) & 0xFF;
                    break;
                default:
                    fprintf(stderr, "Unknown opcode 0x%04X", opcode);
                    return 3;
            }
            break;
        case 0x9000:
            if (N != 0) {
                fprintf(stderr, "Unknown opcode 0x%04X", opcode);
                return 3;
            }

            if (cpu->V[X] != cpu->V[Y]) {
                cpu->pc += 2;
            }
            break;
        case 0xA000:
            cpu->I = NNN;
            break;
        case 0xB000:
            cpu->pc = NNN + cpu->V[0];
            break;
        case 0xC000:
            cpu->V[X] = (int)floor(((double)rand() / RAND_MAX) * 256) & NN;
            break;
        case 0xD000:
            cpu->V[0xF] = 0;
            for (size_t row = 0; row < N; ++row) {
                int sprite_byte = cpu->memory[cpu->I + row];
                for (size_t col = 0; col < 8; ++col) {
                    int mask = 0x80 >> col;
                    if ((sprite_byte & mask) != 0) {
                        int px = (cpu->V[X] + col) & 64;
                        int py = (cpu->V[Y] + row) & 32;
                        int idx = py * 64 + px;
                        if (cpu->display[idx]) {
                            cpu->V[0xF] = 1;
                        }
                        cpu->display[idx] ^= 1;
                    }
                }
            }
            break;
    }
}

void cycle(chip8_cpu *cpu) {
    int opcode = (cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];
    cpu->pc += 2;
    execute_opcode(cpu, opcode);
}