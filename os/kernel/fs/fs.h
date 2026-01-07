#pragma once
#include <stddef.h>

typedef enum {
    FS_FILE,
    FS_DIR
} fs_node_type_t;

typedef struct fs_node {
    const char *name;
    fs_node_type_t type;

    // Directories
    struct fs_node **children;
    size_t child_count;

    // Files
    const char *content;
    size_t size;
} fs_node_t;

void fs_init(void);
void fs_ls(const char *path);
void fs_cat(const char *path);