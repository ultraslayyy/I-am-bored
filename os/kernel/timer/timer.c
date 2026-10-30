#include <arch.h>
#include <io/keyboard.h>
#include <io/kernel_io.h>

static volatile uint64_t ticks = 0;

void timer_tick(void) {
    ticks++;
    keyboard_timer_tick();
}

uint64_t timer_ticks(void) {
    return ticks;
}

uint64_t timer_ms(void) {
    return ticks * (1000 / PIT_INIT_FREQ);
}

void delay_ms(uint32_t ms) {
    uint64_t start = ticks;
    uint64_t rticks = (ms * PIT_INIT_FREQ) / 1000;

    while ((ticks - start) < rticks) {
        __asm__ volatile("hlt");
    } 
}