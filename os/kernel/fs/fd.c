#include <lib/string.h>
#include "fd.h"

file_descriptor_t fd_table[MAX_FD];

void fd_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
}

int fd_open(const char *path) {
    fs_node_t *node = vfs_resolve(path);
    if (!node) {
        return -1;
    }

    for (size_t i = 0; i < MAX_FD; ++i) {
        if (!fd_table[i].used) {
            fd_table[i].used = 1;
            fd_table[i].node = node;
            fd_table[i].offset = 0;
            return i;
        }
    }

    return -1;
}

int fd_remove(const char *path, int recursive) {
    fs_node_t *node;

    if (path[0] == '/') {
        node = vfs_resolve(path);
    } else {
        node = vfs_resolve_from(kernel_cwd, path);
    }

    if (!node) {
        return -1;
    }

    for (size_t i = 0; i < MAX_FD; ++i) {
        if (fd_table[i].used && fd_table[i].node == node) {
            return -2;
        }
    }

    return vfs_rm(path, recursive);
}

size_t fd_read(int fd, char *buffer, size_t len) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used) {
        return 0;
    }

    file_descriptor_t *f = &fd_table[fd];

    size_t read = vfs_read(f->node, f->offset, buffer, len);
    f->offset += read;

    return read;
}

size_t fd_write(int fd, const char *buffer, size_t len) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used) {
        return 0;
    }

    file_descriptor_t *f = &fd_table[fd];

    size_t written = vfs_write(f->node, f->offset, buffer, len);
    f->offset += written;

    return written;
}

void fd_close(int fd) {
    if (fd >= 0 && fd < MAX_FD) {
        fd_table[fd].used = 0;
    }
}