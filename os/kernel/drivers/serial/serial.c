#include <arch.h>
#include <lib/stdint.h>

#define COM1        0x3F8

#define SERIAL_DATA 0
#define SERIAL_IER  1
#define SERIAL_FIFO 2
#define SERIAL_LCR  3
#define SERIAL_MCR  4
#define SERIAL_LSR  5

#define LSR_THRE    0x20

void serial_init(void) {
    // Disable interrupts
    outb(COM1 + SERIAL_IER, 0x00);

    // Enable DLAB
    outb(COM1 + SERIAL_LCR, 0x80);

    // 115200 baud
    outb(COM1 + 0, 0x01);
    outb(COM1 + 1, 0x00);

    outb(COM1 + SERIAL_LCR, 0x03);

    outb(COM1 + SERIAL_FIFO, 0xC7);

    outb(COM1 + SERIAL_MCR, 0x03);
}

void serial_putc(char c) {
    while (!(inb(COM1 + SERIAL_LSR) & LSR_THRE));
    outb(COM1 + SERIAL_DATA, (uint8_t)c);
}

void serial_write(const char *str) {
    while (*str) {
        if (*str == '\n') {
            serial_putc('\r');
        }

        serial_putc(*str++);
    }
}

void serial_write_hex(uint64_t value) {
    static const char hex[] = "0123456789ABCDEF";

    serial_write("0x");

    for (int i = 15; i >= 0; --i) {
        serial_putc(hex[(value >> (i * 4)) & 0xF]);
    }
}

void serial_write_dec(uint32_t value) {
    char buf[11];
    int i = 10;

    buf[i] = '\0';

    if (value == 0) {
        serial_putc('0');
        return;
    }

    while (value > 0) {
        buf[--i] = '0' + (value % 10);
        value /= 10;
    }

    serial_write(&buf[i]);
}