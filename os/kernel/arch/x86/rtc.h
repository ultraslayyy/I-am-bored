#pragma once
#include <lib/stdint.h>

uint8_t rtc_read(uint8_t reg);
void rtc_init(void);
void rtc_get_time(uint8_t *hour, uint8_t *minute, uint8_t *second);