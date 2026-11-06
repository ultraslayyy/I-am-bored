#pragma once
#include <lib/stdint.h>

static inline uint32_t irq_save(void) {
    uint32_t flags;
    __asm__ volatile("pushfl\n\t" "pop %0\n\t" "cli" : "=r"(flags) : : "memory");
    return flags;
}

static inline void irq_restore(uint32_t flags) {
    if (flags & 0x200) {
        __asm__ volatile("sti" ::: "memory");
    }
}