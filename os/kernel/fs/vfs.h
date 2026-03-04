#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>

#define MAX_PATH_LEN 256

typedef enum {
    FS_FILE,
    FS_DIR
} fs_node_type_t;

typedef struct fs_node {
    char name[64];
    fs_node_type_t type;

    struct fs_node *parent;

    struct fs_node **children;
    size_t child_count;
    size_t child_capacity;

    uint8_t *data;
    size_t size;
    size_t capacity;

    void *private;
} fs_node_t;

extern fs_node_t *kernel_cwd;

typedef struct filesystem {
    fs_node_t *root;

    fs_node_t *(*resolve)(struct filesystem *fs, const char *path);
    fs_node_t *(*resolve_from)(struct filesystem *fs, fs_node_t *start, const char *path);
    fs_node_t *(*create)(struct filesystem *fs, fs_node_t *parent, const char *name, fs_node_type_t type);
    int (*remove)(struct filesystem *fs, fs_node_t *node, int recursive); // Possibly make bitwise shift based for mode
    size_t (*read)(fs_node_t *node, size_t offset, char *buffer, size_t len);
    size_t (*write)(fs_node_t *node, size_t offset, const char *buffer, size_t len);
} filesystem_t;

void vfs_init(void);
void vfs_mount_root(filesystem_t *fs);
fs_node_t *vfs_resolve(const char *path);
fs_node_t *vfs_resolve_from(fs_node_t *start, const char *path);
fs_node_t *vfs_create(const char *path, fs_node_type_t type);
fs_node_t *vfs_create_from(fs_node_t *start, const char *path, fs_node_type_t type);
int vfs_rm(const char *path, int recursive);
size_t vfs_read(fs_node_t *node, size_t offset, char *buffer, size_t len);
size_t vfs_write(fs_node_t *node, size_t offset, const char *buffer, size_t len);
int vfs_chdir(const char *path);
void vfs_get_path(fs_node_t *node, char *buffer, size_t max);