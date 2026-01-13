#pragma once
#include <lib/stddef.h>

typedef enum {
    FS_FILE,
    FS_DIR
} fs_node_type_t;

typedef size_t (*fs_read_fn)(char *buffer, size_t max);

typedef struct fs_node {
    const char *name;
    fs_node_type_t type;

    // Directories
    struct fs_node **children;
    size_t child_count;

    // Files (static)
    const char *content;
    size_t size;

    // Files (dynamic)
    fs_read_fn read;
} fs_node_t;

void fs_init(void);
void fs_ls(const char *path);
void fs_cat(const char *path);
int cmd_ls(int argc, char **argv);
int cmd_cat(int argc, char **argv);