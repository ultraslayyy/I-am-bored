#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include "shell.h"

int cmd_echo(int argc, char **argv) {
    if (argc < 2) return 1;

    int redirect_index = -1;
    for (size_t i = 1; i < argc; ++i) {
        if (strcmp(argv[i], ">") == 0) {
            redirect_index = i;
            break;
        }
    }

    char buf[MAX_INPUT];
    size_t len = 0;
    buf[0] = '\0';

    if (redirect_index == -1) {
        for (size_t i = 1; i < argc; ++i) {
            len += snprintf(buf + len, sizeof(buf) - len, "%s%s", argv[i], (i + 1 < argc ? " " : ""));
        }
        put_string(buf, DEFAULT_ATTR);
        put_char('\n', DEFAULT_ATTR);
        return 0;
    }

    for (size_t i = 1; i < redirect_index; ++i) {
        len += snprintf(buf + len, sizeof(buf) - len, "%s%s", argv[i], (i + 1 < redirect_index ? " " : ""));
    }

    if (redirect_index + 1 >= argc) {
        put_string("Usage: echo text > file\n", DEFAULT_ATTR);
        return 1;
    }

    const char *filename = argv[redirect_index + 1];

    fs_node_t *file;
    if (filename[0] == '/') {
        file = vfs_resolve(filename);
    } else {
        file = vfs_resolve_from(kernel_cwd, filename);
    }

    if (!file) {
        file = vfs_create_from(kernel_cwd, filename, FS_FILE);
        if (!file) {
            put_string("Failed to create file\n", DEFAULT_ATTR);
            return 1;
        }
    }

    file->size = 0;
    vfs_write(file, 0, buf, strlen(buf));

    return 0;
}