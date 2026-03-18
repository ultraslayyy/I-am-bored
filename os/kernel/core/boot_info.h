#pragma once
#include <lib/stdint.h>

#define BOOT_MMAP_MAX 128

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
} boot_mmap_entry_t;

typedef struct {
    uint64_t mem_lower;
    uint64_t mem_upper;

    uint32_t flags;
    boot_mmap_entry_t mmap[BOOT_MMAP_MAX];
    uint32_t mmap_entries;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
} boot_info_t;
