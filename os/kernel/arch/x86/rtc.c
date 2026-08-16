#include "io.h"
#include "rtc.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

#define RTC_SECONDS  0x00
#define RTC_MINUTES  0x02
#define RTC_HOURS    0x04
#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B
#define RTC_STATUS_C 0x0C

static uint8_t rtc_reg_b;

uint8_t rtc_read(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

static uint8_t bcd_to_bin(uint8_t value) {
    return (value & 0x0F) + ((value >> 4) * 10);
}

static int rtc_update_in_progress(void) {
    return rtc_read(RTC_STATUS_A) & 0x80;
}

void rtc_init(void) {
    rtc_reg_b = rtc_read(RTC_STATUS_B);
}

void rtc_get_time(uint8_t *hour, uint8_t *minute, uint8_t *second) {
    while (rtc_update_in_progress());

    uint8_t sec = rtc_read(RTC_SECONDS);
    uint8_t min = rtc_read(RTC_MINUTES);
    uint8_t hr  = rtc_read(RTC_HOURS);

    // If bit 2 of Reg B is 0 then BCD, else binary
    if (!(rtc_reg_b & 0x04)) {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hr  = bcd_to_bin(hr);
    }

    *hour   = hr;
    *minute = min;
    *second = sec;
}