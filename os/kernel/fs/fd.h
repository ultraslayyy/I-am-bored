#pragma once
#include "vfs.h"

#define MAX_FD 64

typedef struct {
    fs_node_t *node;
    size_t offset;
    int used;
} file_descriptor_t;

extern file_descriptor_t fd_table[MAX_FD];

void fd_init(void);
int fd_open(const char *path);
int fd_remove(const char *path, int recursive);
size_t fd_read(int fd, char *buffer, size_t len);
size_t fd_write(int fd, const char *buffer, size_t len);
void fd_close(int fd);