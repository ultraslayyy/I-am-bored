#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <lib/string.h>

int cmd_cd(int argc, char **argv) {
    if (argc != 2) {
        put_string("Usage: cd <dir>\n", DEFAULT_ATTR);
        return 1;
    }

    if (vfs_chdir(argv[1]) != 0) {
        put_string("Not a directory or invalid path\n", DEFAULT_ATTR);
        return 1;
    }

    return 0;
}