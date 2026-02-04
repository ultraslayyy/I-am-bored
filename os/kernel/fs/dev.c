#include "dev.h"

static size_t null_read(char *buffer, size_t max) {
    (void)buffer;
    (void)max;
    return 0;
}

static size_t null_write(const char *buffer, size_t len) {
    (void)buffer;
    return len;
}

static fs_node_t file_null = {
    .name  = "null",
    .type  = FS_FILE,
    .read  = null_read,
    .write = null_write
};

static fs_node_t *dev_children[] = {
    &file_null
};

fs_node_t dir_dev = {
    .name = "dev",
    .type = FS_DIR,
    .children = dev_children,
    .child_count = 1
};