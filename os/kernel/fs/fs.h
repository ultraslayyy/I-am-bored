#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>

#define MAX_PATH_LEN 256

typedef enum {
    FS_FILE,
    FS_DIR
} fs_node_type_t;

typedef size_t (*fs_read_fn)(char *buffer, size_t max);
typedef size_t(*fs_write_fn)(const char *buffer, size_t len);

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
    fs_read_fn  read;
    fs_write_fn write;

    const int8_t elevated;
} fs_node_t;

void fs_init(void);
void fs_ls(const char *path);
void fs_cat(const char *path);
fs_node_t *fs_find(const char *path);
int resolve_path(char out[MAX_PATH_LEN], const char *in);
int cmd_ls(int argc, char **argv);
int cmd_cat(int argc, char **argv);
size_t fs_read_file(fs_node_t *node, char *buffer, size_t max_len);