#include <arch.h>
#include "font.h"
#include "vesa.h"

uint32_t vesa_width = 0;
uint32_t vesa_height = 0;
uint32_t vesa_pitch = 0;
uint8_t vesa_bpp = 0;
uint64_t vesa_framebuffer = 0;

void vesa_init(boot_info_t *info) {
    vesa_width = info->framebuffer_width;
    vesa_height = info->framebuffer_height;
    vesa_pitch = info->framebuffer_pitch;
    vesa_bpp = info->framebuffer_bpp;
    vesa_framebuffer = info->framebuffer_addr;

    uint32_t fb_size = vesa_height * vesa_pitch;
    uint32_t fb_pages = (fb_size + 0xFFF) / PAGE_SIZE;
    
    extern void map_page(uint32_t virt, uint32_t phys, uint32_t flags);
    
    for (uint32_t i = 0; i < fb_pages; ++i) {
        uint32_t base = vesa_framebuffer & 0xFFFFF000;
        map_page(base + i * PAGE_SIZE, base + i * PAGE_SIZE, PAGE_PRESENT | PAGE_RW);
    }
}