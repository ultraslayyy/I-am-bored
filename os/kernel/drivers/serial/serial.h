#pragma once
#include <lib/stdint.h>

void serial_init(void);

void serial_putc(char c);
void serial_write(const char *str);

void serial_write_hex(uint64_t value);
void serial_write_dec(uint32_t value);