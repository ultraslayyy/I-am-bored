#include <lib/stdlib.h>
#include <lib/string.h>
#include "ramfs.h"

static fs_node_t ramfs_root;

static fs_node_t *ramfs_resolve(filesystem_t *fs, const char *path);
static fs_node_t *ramfs_resolve_from(filesystem_t *fs, fs_node_t *start, const char *path);
static fs_node_t *ramfs_create(filesystem_t *fs, fs_node_t *parent, const char *name, fs_node_type_t type);
static int ramfs_remove(filesystem_t *fs, fs_node_t *node, int recursive);
static size_t ramfs_read(fs_node_t *node, size_t offset, char *buffer, size_t len);
static size_t ramfs_write(fs_node_t *node, size_t offset, const char *buffer, size_t len);

filesystem_t ramfs = {
    .root = &ramfs_root,
    .resolve = ramfs_resolve,
    .resolve_from = ramfs_resolve_from,
    .create = ramfs_create,
    .read = ramfs_read,
    .write = ramfs_write,
    .remove = ramfs_remove
};

void ramfs_init(void) {
    memset(&ramfs_root, 0, sizeof(fs_node_t));
    strcpy(ramfs_root.name, "/");
    ramfs_root.type = FS_DIR;
    ramfs_root.parent = NULL;
    ramfs_root.children = NULL;
    ramfs_root.child_count = 0;
    ramfs_root.child_capacity = 0;

    vfs_mount_root(&ramfs);
}

static void ramfs_add_child(fs_node_t *parent, fs_node_t *child) {
    if (!parent || !child) return;

    if (parent->child_count >= parent->child_capacity) {
        size_t new_cap = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;

        if (!parent->children) {
            parent->children = malloc(new_cap * sizeof(fs_node_t *));
        } else { 
            fs_node_t **new_children = realloc(parent->children, new_cap * sizeof(fs_node_t *));
            if (!new_children) return;
            parent->children = new_children;
        }

        parent->child_capacity = new_cap;
    }

    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

static fs_node_t *ramfs_find_child(fs_node_t *dir, const char *name) {
    for (size_t i = 0; i < dir->child_count; ++i) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    return 0;
}

static void ramfs_remove_child(fs_node_t *parent, fs_node_t *child) {
    if (!parent || !child || parent->child_count == 0) return;

    size_t index = parent->child_count;
    for (size_t i = 0; i < parent->child_count; ++i) {
        if (parent->children[i] == child) {
            index = i;
            break;
        }
    }

    if (index == parent->child_count) return;

    for (size_t i = index; i < parent->child_count - 1; ++i) {
        parent->children[i] = parent->children[i + 1];
    }

    parent->child_count--;
}

static fs_node_t *ramfs_resolve(filesystem_t *fs, const char *path) {
    return ramfs_resolve_from(fs, fs->root, path);
}

static fs_node_t *ramfs_resolve_from(filesystem_t *fs, fs_node_t *start, const char *path) {
    if (!fs || !start || !path) {
        return 0;
    }

    fs_node_t *current;

    if (path[0] == '/') {
        current = fs->root;
    } else {
        current = start;
    }

    while (*path == '/') path++;

    if (*path == '\0') {
        return current;
    }

    char temp[MAX_PATH_LEN];
    strlcpy(temp, path, sizeof(temp));

    char *temp_ptr = temp;
    char *token = strktok(&temp_ptr, "/");

    while (token) {
        if (strcmp(token, ".") == 0) {
            token = strktok(&temp_ptr, "/");
            continue;
        }

        if (strcmp(token, "..") == 0) {
            if (current->parent) {
                current = current->parent;
            }

            token = strktok(&temp_ptr, "/");
            continue;
        }

        if (current->type != FS_DIR) {
            return 0;
        }

        fs_node_t *next = ramfs_find_child(current, token);
        if (!next) {
            return 0;
        }

        current = next;
        token = strktok(&temp_ptr, "/");
    }

    return current;
}

static fs_node_t *ramfs_create(filesystem_t *fs, fs_node_t *parent, const char *name, fs_node_type_t type) {
    (void)fs;
    
    if (!parent || parent->type != FS_DIR) {
        return 0;
    }

    fs_node_t *node = malloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));

    strlcpy(node->name, name, sizeof(node->name));
    node->type = type;

    ramfs_add_child(parent, node);
    return node;
}

static void ramfs_remove_recursive_node(fs_node_t *node) {
    if (!node) return;

    if (node->type == FS_DIR) {
        while (node->child_count > 0) {
            fs_node_t *child = node->children[node->child_count - 1];
            ramfs_remove_recursive_node(child);
        }
    }

    if (node->type == FS_FILE && node->data) {
        free(node->data);
        node->data = NULL;
    }

    if (node->parent) {
        ramfs_remove_child(node->parent, node);
    }

    free(node);
}

static int ramfs_remove(filesystem_t *fs, fs_node_t *node, int recursive) {
    (void)fs;

    if (!node || !node->parent) return -1;

    if (node->type == FS_DIR && node->child_count > 0 && !recursive) {
        return -2;
    }

    if (recursive) {
        ramfs_remove_recursive_node(node);
        return 0;
    }

    if (node->type == FS_FILE && node->data) {
        free(node->data);
        node->data = NULL;
    }

    ramfs_remove_child(node->parent, node);
    free(node);
    return 0;
}

static size_t ramfs_read(fs_node_t *node, size_t offset, char *buffer, size_t len) {
    if (!node || node->type != FS_FILE) {
        return 0;
    }

    if (offset >= node->size) {
        return 0;
    }

    size_t remaining = node->size - offset;
    size_t to_read = len < remaining ? len : remaining;

    memcpy(buffer, node->data + offset, to_read);
    return to_read;
}

static size_t ramfs_write(fs_node_t *node, size_t offset, const char *buffer, size_t len) {
    if (!node || node->type != FS_FILE) {
        return 0;
    }

    size_t required = offset + len;
    if (required > node->capacity) {
        size_t new_cap = required * 2;
        void *new_data = realloc(node->data, new_cap);
        if (!new_data) return 0;

        node->data = new_data;
        node->capacity = new_cap;
    }

    memcpy(node->data + offset, buffer, len);

    if (required > node->size) {
        node->size = required;
    }

    return len;
}