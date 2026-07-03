#include <drivers/mouse/ps2.h>
#include <io/kernel_io.h>
#include <lib/string.h>

int cmd_mouse(int argc, char **argv) {
    if (argc < 2) {
        put_string("Usage: mouse <enable|disable>\n", DEFAULT_ATTR);
        return 1;
    }

    if (strcmp(argv[1], "enable") == 0) {
        mouse_set_enabled(1);
        return 0;
    } else if (strcmp(argv[1], "disable") == 0) {
        mouse_set_enabled(0);
        return 0;
    }

    put_string("Usage: mouse <enable|disable>\n", DEFAULT_ATTR);
    return 1;
}