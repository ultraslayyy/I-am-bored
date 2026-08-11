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