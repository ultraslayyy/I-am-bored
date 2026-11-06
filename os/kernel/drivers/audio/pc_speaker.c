#include <timer/timer.h>
#include <arch.h>

#define PIT_CHANNEL2 0x42
#define PIT_COMMAND  0x43
#define SPEAKER_PORT 0x61

void speaker_play(uint32_t frequency) {
    if (frequency == 0) {
        return;
    }

    uint32_t divisor = PIT_BASE_FREQ / frequency;

    outb(PIT_COMMAND, 0xB6); // square wave mode

    outb(PIT_CHANNEL2, divisor & 0xFF);
    outb(PIT_CHANNEL2, (divisor >> 8) & 0xFF);

    uint8_t value = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, value | 0x03);
}

void speaker_stop(void) {
    uint8_t value = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, value & ~0x03);
}

void speaker_beep(uint32_t frequency, uint32_t duration_ms) {
    speaker_play(frequency);
    delay_ms(duration_ms);
    speaker_stop();
}

void speaker_init(void) {
    speaker_stop();
}