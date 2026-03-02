#include <fs/vfs.h>
#include <io/kernel_io.h>

int cmd_mkdir(int argc, char **argv) {
    if (argc != 2) {
        put_string("Usage: mkdir <dir>\n", DEFAULT_ATTR);
        return 1;
    }

    fs_node_t *node;
    if (argv[1][0] == '/') {
        node = vfs_create(argv[1], FS_DIR);
    } else {
        node = vfs_create_from(kernel_cwd, argv[1], FS_DIR);
    }

    if (!node) {
        put_string("Failed to create directory\n", DEFAULT_ATTR);
        return 1;
    }

    return 0;
}