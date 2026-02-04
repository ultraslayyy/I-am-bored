#include <arch.h>

int cmd_shutdown(int argc, char **argv) {
    (void)argc;
    (void)argv;

    outw(0x604, 0x2000);
    return 0;
}