#include <lib/string.h>
#include "byteorder.h"

uint32_t parse_ip(const char *str) {
    uint32_t ip = 0;
    int octet = 0;
    int shift = 24; // MSB
    const char *p = str;

    while (*p) {
        if (*p >= '0' && *p <= '9') {
            octet = octet * 10 + (*p - '0');
        } else if (*p == '.') {
            ip |= (octet & 0xFF) << shift;
            shift -= 8;
            octet = 0;
        } else {
            return 0;
        }
        p++;
    }
    ip |= (octet & 0xFF);
    return htonl(ip);
}
