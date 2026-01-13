#include <arch.h>

void cmd_shutdown(int argc, char **argv) {
    outw(0x604, 0x2000);
}