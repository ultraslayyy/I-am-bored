#include <drivers/mouse/ps2.h>
#include <io/kernel_io.h>
#include <lib/string.h>

int cmd_mouse(int argc, char **argv) {
    if (argc < 2) {
        put_string("Usage: mouse <enable|disable>\n", DEFAULT_ATTR);
        return 1;
    }

    int ec = 0;
    if (strcmp(argv[1], "enable") == 0) {
        ec = mouse_set_enabled(1);
        if (ec == 0) {
            put_string("Mouse doesn't exist.\n", DEFAULT_ATTR);
        }
        return ec;
    } else if (strcmp(argv[1], "disable") == 0) {
        ec = mouse_set_enabled(0);
        if (ec == 0) {
            put_string("Mouse doesn't exist.\n", DEFAULT_ATTR);
        }
        return ec;
    }

    put_string("Usage: mouse <enable|disable>\n", DEFAULT_ATTR);
    return 1;
}