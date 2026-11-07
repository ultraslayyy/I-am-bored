#pragma once

#include <core/boot_info.h>
#include <lib/stdint.h>

void vesa_init(boot_info_t *info);

extern uint32_t vesa_width;
extern uint32_t vesa_height;
extern uint32_t vesa_pitch;
extern uint8_t vesa_bpp;
extern uint64_t vesa_framebuffer;