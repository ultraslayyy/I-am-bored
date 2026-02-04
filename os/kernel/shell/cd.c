#include <fs/cwd.h>
#include <fs/fs.h>
#include <io/kernel_io.h>
#include <lib/string.h>

int cmd_cd(int argc, char **argv) {
    if (argc != 2) {
        put_string("Usage: cd <dir>\n", DEFAULT_ATTR);
        return 1;
    }

    char path[MAX_PATH_LEN];
    if (resolve_path(path, argv[1]) != 0) {
        put_string("Invalid path\n", DEFAULT_ATTR);
        return 1;
    }

    fs_node_t *node = fs_find(path);
    if (!node || node->type != FS_DIR) {
        put_string("Not a directory\n", DEFAULT_ATTR);
        return 1;
    }

    strlcpy(g_cwd, path, MAX_PATH_LEN);

    return 0;
}