#include <arch.h>
#include "bochs.h"

uint32_t bochs_vbe_width       = 0;
uint32_t bochs_vbe_height      = 0;
uint32_t bochs_vbe_pitch       = 0;
uint8_t  bochs_vbe_bpp         = 0;
uint64_t bochs_vbe_framebuffer = 0;

static void bochs_vbe_write(uint16_t index, uint16_t value) {
    outw(BOCHS_VBE_INDEX_PORT, index);
    outw(BOCHS_VBE_DATA_PORT, value);
}

static uint16_t bochs_vbe_read(uint16_t index) {
    outw(BOCHS_VBE_INDEX_PORT, index);
    return inw(BOCHS_VBE_DATA_PORT);
}

static int bochs_vbe_available(void) {
    uint16_t id = bochs_vbe_read(BOCHS_VBE_INDEX_ID);

    return id >= BOCHS_VBE_ID0 && id <= BOCHS_VBE_ID5;
}

static int bochs_vbe_set_mode(uint32_t width, uint32_t height, uint32_t bpp) {
    bochs_vbe_write(BOCHS_VBE_INDEX_ENABLE, BOCHS_VBE_DISABLED);

    bochs_vbe_write(BOCHS_VBE_INDEX_XRES, (uint16_t)width);
    bochs_vbe_write(BOCHS_VBE_INDEX_YRES, (uint16_t)height);
    bochs_vbe_write(BOCHS_VBE_INDEX_BPP, (uint16_t)bpp);

    bochs_vbe_write(BOCHS_VBE_INDEX_ENABLE, BOCHS_VBE_ENABLED | BOCHS_VBE_LFB_ENABLED | BOCHS_VBE_NOCLEARMEM);

    return 0;
}

int bochs_vbe_init(void) {
    if (!bochs_vbe_available()) {
        return -1;
    }

    if (bochs_vbe_set_mode(BOCHS_VBE_WIDTH, BOCHS_VBE_HEIGHT, BOCHS_VBE_BPP) != 0) {
        return -1;
    }

    bochs_vbe_width  = BOCHS_VBE_WIDTH;
    bochs_vbe_height = BOCHS_VBE_HEIGHT;
    bochs_vbe_bpp    = BOCHS_VBE_BPP;
    bochs_vbe_pitch  = bochs_vbe_width * (bochs_vbe_bpp / 8);
    bochs_vbe_framebuffer = BOCHS_VBE_LFB_ADDRESS;

    uint32_t fb_size  = bochs_vbe_height * bochs_vbe_pitch;
    uint32_t fb_pages = (fb_size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < fb_pages; ++i) {
        uint32_t addr = BOCHS_VBE_LFB_ADDRESS + i * PAGE_SIZE;

        map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }

    return 0;
}

void bochs_vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= bochs_vbe_width || y >= bochs_vbe_height) {
        return;
    }

    uint32_t *fb = (uint32_t *)(bochs_vbe_framebuffer + y * bochs_vbe_pitch + x * 4);
    *fb = color;
}

void bochs_vbe_clear_screen(uint32_t color) {
    for (uint32_t y = 0; y < bochs_vbe_height; ++y) {
        for (uint32_t x = 0; x <= bochs_vbe_width; ++x) {
            bochs_vbe_draw_pixel(x, y, color);
        }
    }
}