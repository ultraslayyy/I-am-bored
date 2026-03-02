#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <lib/string.h>

int cmd_rm(int argc, char **argv) {
    int recursive = 0;
    const char *target = 0;

    if (argc == 2) {
        target = argv[1];
    } else if (argc == 3 && strcmp(argv[1], "-r") == 0) {
        recursive = 1;
        target = argv[2];
    } else if  (argc != 2) {
        put_string("Usage: rm [-r] <file>\n", DEFAULT_ATTR);
        return 1;
    }

    int res = vfs_rm(target, recursive);
    if (res != 0) {
        if (res == -2) {
            put_string("Directory not empty (use -r)\n", DEFAULT_ATTR);
        } else {
            put_string("Failed to remove\n", DEFAULT_ATTR);
        }
        return 1;
    }

    return 0;
}