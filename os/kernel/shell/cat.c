#include <fs/vfs.h>
#include <io/kernel_io.h>

int cmd_cat(int argc, char **argv) {
    if (argc != 2) {
        put_string("Usage: cat <file>\n", DEFAULT_ATTR);
        return 1;
    }

    fs_node_t *node;

    if (argv[1][0] == '/') {
        node = vfs_resolve(argv[1]);
    } else {
        node = vfs_resolve_from(kernel_cwd, argv[1]);
    }

    if (!node || node->type != FS_FILE) {
        put_string("Not a file\n", DEFAULT_ATTR);
        return 1;
    }

    if (node->size == 0) {
        put_string("(empty)\n", DEFAULT_ATTR);
        return 0;
    }

    char buffer[256];
    size_t offset = 0;

    while (offset < node->size) {
        size_t read = vfs_read(node, offset, buffer, sizeof(buffer));
        if (!read) break;

        for (size_t i = 0; i < read; ++i) {
            put_char(buffer[i], DEFAULT_ATTR);
        }

        offset += read;
    }

    put_char('\n', DEFAULT_ATTR);
    return 0;
}