#pragma once

static inline void disable_interrupts() {
    __asm__ volatile("cli");
}

static inline void enable_interrupts() {
    __asm__ volatile("sti");
}