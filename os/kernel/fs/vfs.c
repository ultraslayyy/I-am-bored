#include <lib/string.h>
#include "vfs.h"

static filesystem_t *root_fs = 0;
fs_node_t *kernel_cwd = 0;

void vfs_init(void) {
    root_fs = 0;
    kernel_cwd = 0;
}

void vfs_mount_root(filesystem_t *fs) {
    root_fs = fs;
    if (fs) {
        kernel_cwd = fs->root;
    }
}

fs_node_t *vfs_resolve(const char *path) {
    if (!root_fs || !path) {
        return 0;
    }

    return root_fs->resolve(root_fs, path);
}

fs_node_t *vfs_resolve_from(fs_node_t *start, const char *path) {
    if (!root_fs || !path || !start) {
        return 0;
    }

    return root_fs->resolve_from(root_fs, start, path);
}

fs_node_t *vfs_create(const char *path, fs_node_type_t type) {
    if (!root_fs || !path) {
        return 0;
    }

    char temp[MAX_PATH_LEN];
    strlcpy(temp, path, MAX_PATH_LEN);

    char *last = strrchr(temp, '/');
    if (!last) {
        return 0;
    }

    if (last == temp) {
        return root_fs->create(root_fs, root_fs->root, last + 1, type);
    }

    *last = 0;
    fs_node_t *parent = root_fs->resolve(root_fs, temp);
    if (!parent) {
        return 0;
    }

    return root_fs->create(root_fs, parent, last + 1, type);
}

fs_node_t *vfs_create_from(fs_node_t *start, const char *path, fs_node_type_t type) {
    if (!root_fs || !path) {
        return 0;
    }

    char temp[MAX_PATH_LEN];
    strlcpy(temp, path, MAX_PATH_LEN);

    fs_node_t *base;
    if (temp[0] == '/') {
        base = root_fs->root;
    } else {
        base = start;
    }

    char *last = strrchr(temp, '/');
    if (!last) {
        return root_fs->create(root_fs, base, temp, type);
    }

    if (last == temp) {
        return root_fs->create(root_fs, root_fs->root, last + 1, type);
    }

    *last = 0;
    fs_node_t *parent;
    if (temp[0] == '/') {
        parent = root_fs->resolve(root_fs, temp);
    } else {
        parent = root_fs->resolve_from(root_fs, start, temp);
    }

    if (!parent) {
        return 0;
    }

    return root_fs->create(root_fs, parent, last + 1, type);
}

int vfs_rm(const char *path, int recursive) {
    if (!root_fs || !path) return -1;

    fs_node_t *node;
    if (path[0] == '/') {
        node = vfs_resolve(path);
    } else {
        node = vfs_resolve_from(kernel_cwd, path);
    }

    if (!node) return -1;

    return root_fs->remove(root_fs, node, recursive);
}

size_t vfs_read(fs_node_t *node, size_t offset, char *buffer, size_t len) {
    if (!root_fs) {
        return 0;
    }
    return root_fs->read(node, offset, buffer, len);
}

size_t vfs_write(fs_node_t *node, size_t offset, const char *buffer, size_t len) {
    if (!root_fs) {
        return 0;
    }
    return root_fs->write(node, offset, buffer, len);
}

int vfs_chdir(const char *path) {
    if (!path) {
        return -1;
    }

    fs_node_t *node;

    if (path[0] == '/') {
        node = vfs_resolve(path);
    } else {
        node = vfs_resolve_from(kernel_cwd, path);
    }

    if (!node || node->type != FS_DIR) {
        return -1;
    }

    kernel_cwd = node;
    return 0;
}

void vfs_get_path(fs_node_t *node, char *buffer, size_t max) {
    if (!node || !buffer || max == 0) return;

    if (!node->parent) {
        strlcpy(buffer, "/", max);
        return;
    }

    char temp[MAX_PATH_LEN];
    size_t pos = MAX_PATH_LEN - 1;
    temp[pos] = 0;

    fs_node_t *cur = node;
    while (cur && cur->parent) {
        if (cur == cur->parent) break;

        size_t len = strlen(cur->name);
        if (len + 1 > pos) break;

        pos -= len;
        memcpy(temp + pos, cur->name, len);

        pos--;
        temp[pos] = '/';

        cur = cur->parent;
    }

    strlcpy(buffer, temp + pos, max);
}