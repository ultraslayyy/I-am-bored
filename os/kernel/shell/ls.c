#include <fs/vfs.h>
#include <io/kernel_io.h>

int cmd_ls(int argc, char **argv) {
    fs_node_t *node;

    if (argc == 1) {
        node = kernel_cwd;
    } else if (argc == 2) {
        if (argv[1][0] == '/') {
            node = vfs_resolve(argv[1]);
        } else {
            node = vfs_resolve_from(kernel_cwd, argv[1]);
        }

        if (!node) {
            put_string("Invalid path\n", DEFAULT_ATTR);
            return 1;
        }
    } else {
        put_string("Too many parameters\n", DEFAULT_ATTR);
        return 1;
    }

    if (node->type != FS_DIR) {
        put_string("Not a directorty\n", DEFAULT_ATTR);
        return 1;
    }

    for (size_t i = 0; i < node->child_count; ++i) {
        put_string(node->children[i]->name, DEFAULT_ATTR);

        if (node->children[i]->type == FS_DIR) {
            put_string("/", DEFAULT_ATTR);
        }

        put_string("  ", DEFAULT_ATTR);
    }

    put_char('\n', DEFAULT_ATTR);

    return 0;
}