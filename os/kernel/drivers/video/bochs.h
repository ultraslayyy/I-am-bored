#pragma once
#include <lib/stdint.h>

#define BOCHS_VBE_INDEX_PORT 0x01CE
#define BOCHS_VBE_DATA_PORT  0x01CF

#define BOCHS_VBE_INDEX_ID           0x00
#define BOCHS_VBE_INDEX_XRES         0x01
#define BOCHS_VBE_INDEX_YRES         0x02
#define BOCHS_VBE_INDEX_BPP          0x03
#define BOCHS_VBE_INDEX_ENABLE       0x04
#define BOCHS_VBE_INDEX_BANK         0x05
#define BOCHS_VBE_INDEX_VIRT_WIDTH   0x06
#define BOCHS_VBE_INDEX_VIRT_HEIGHT  0x07
#define BOCHS_VBE_INDEX_X_OFFSET     0x08
#define BOCHS_VBE_INDEX_Y_OFFSET     0x09
#define BOCHS_VBE_INDEX_VIDEO_MEMORY 0x0A

#define BOCHS_VBE_ID0 0xB0C0
#define BOCHS_VBE_ID1 0xB0C1
#define BOCHS_VBE_ID2 0xB0C2
#define BOCHS_VBE_ID3 0xB0C3
#define BOCHS_VBE_ID4 0xB0C4
#define BOCHS_VBE_ID5 0xB0C5

#define BOCHS_VBE_DISABLED    0x00
#define BOCHS_VBE_ENABLED     0x01
#define BOCHS_VBE_LFB_ENABLED 0x40
#define BOCHS_VBE_NOCLEARMEM  0x80

#define BOCHS_VBE_LFB_ADDRESS 0xE0000000

#define BOCHS_VBE_WIDTH  1024
#define BOCHS_VBE_HEIGHT 768
#define BOCHS_VBE_BPP    32

extern uint32_t bochs_vbe_width;
extern uint32_t bochs_vbe_height;
extern uint32_t bochs_vbe_pitch;
extern uint8_t  bochs_vbe_bpp;
extern uint64_t bochs_vbe_framebuffer;

int bochs_vbe_init(void);
void bochs_vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void bochs_vbe_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
void bochs_vbe_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void bochs_vbe_clear_screen(uint32_t color);