#include <arch.h>

int cmd_shutdown() {
    outw(0x604, 0x2000);
    return 0;
}