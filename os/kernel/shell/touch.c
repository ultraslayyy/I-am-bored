#include <fs/vfs.h>
#include <io/kernel_io.h>

int cmd_touch(int argc, char **argv) {
    if (argc != 2) {
        put_string("Usage: touch <file>\n", DEFAULT_ATTR);
        return 1;
    }

    fs_node_t *node;
    if (argv[1][0] == '/') {
        node = vfs_create(argv[1], FS_FILE);
    } else {
        node = vfs_create_from(kernel_cwd, argv[1], FS_FILE);
    }

    if (!node) {
        put_string("Failed to create file\n", DEFAULT_ATTR);
        return 1;
    }

    return 0;
}