#include <arch.h>
#include <lib/string.h>
#include <io/kernel_io.h>

int cmd_time(int argc, char **argv) {
    (void)argc;
    (void)argv;

    uint8_t hr;
    uint8_t min;
    uint8_t sec;

    rtc_get_time(&hr, &min, &sec);

    char buf[32];
    snprintf(buf, sizeof(buf), "%d:%d:%d\n", hr, min, sec);
    put_string(buf, DEFAULT_ATTR);

    return 0;
}