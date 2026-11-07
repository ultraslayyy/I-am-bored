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
    if (width > 0xFFFF || height > 0xFFFF || bpp > 0xFFFF) {
        return -1;
    }

    bochs_vbe_write(BOCHS_VBE_INDEX_ENABLE, BOCHS_VBE_DISABLED);

    bochs_vbe_write(BOCHS_VBE_INDEX_XRES, (uint16_t)width);
    bochs_vbe_write(BOCHS_VBE_INDEX_YRES, (uint16_t)height);
    bochs_vbe_write(BOCHS_VBE_INDEX_BPP, (uint16_t)bpp);

    bochs_vbe_write(BOCHS_VBE_INDEX_ENABLE, BOCHS_VBE_ENABLED | BOCHS_VBE_LFB_ENABLED | BOCHS_VBE_NOCLEARMEM);

    if (bochs_vbe_read(BOCHS_VBE_INDEX_XRES) != width || bochs_vbe_read(BOCHS_VBE_INDEX_YRES) != height || bochs_vbe_read(BOCHS_VBE_INDEX_BPP) != bpp) {
        bochs_vbe_write(BOCHS_VBE_INDEX_ENABLE, BOCHS_VBE_DISABLED);
        return -1;
    }

    return 0;
}

static void bochs_vbe_map_framebuffer(uint32_t address, uint32_t size) {
    uint32_t base   = address & ~(PAGE_SIZE - 1);
    uint32_t offset = address - base;

    uint32_t total_size = size + offset;
    uint32_t pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages; ++i) {
        uint32_t addr = base + i * PAGE_SIZE;

        map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }
}

int bochs_vbe_init(void) {
    if (!bochs_vbe_available()) {
        return -1;
    }

    if (bochs_vbe_set_mode(BOCHS_VBE_WIDTH, BOCHS_VBE_HEIGHT, BOCHS_VBE_BPP) != 0) {
        return -1;
    }

    bochs_vbe_width       = bochs_vbe_read(BOCHS_VBE_INDEX_XRES);
    bochs_vbe_height      = bochs_vbe_read(BOCHS_VBE_INDEX_YRES);
    bochs_vbe_bpp         = bochs_vbe_read(BOCHS_VBE_INDEX_BPP);
    bochs_vbe_pitch       = bochs_vbe_read(BOCHS_VBE_INDEX_VIRT_WIDTH) * (bochs_vbe_bpp / 8);
    bochs_vbe_framebuffer = BOCHS_VBE_LFB_ADDRESS;

    if (bochs_vbe_bpp != 32) {
        return -1;
    }

    uint32_t fb_size = bochs_vbe_height * bochs_vbe_pitch;
    bochs_vbe_map_framebuffer((uint32_t)bochs_vbe_framebuffer, fb_size);
    return 0;
}