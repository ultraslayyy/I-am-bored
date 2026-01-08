#include "procfs.h"
#include <lib/string.h>
#include <drivers/memory/memory.h>
#include <drivers/memory/iomem.h>

static size_t proc_meminfo_read(char *buffer, size_t max) {
    return snprintf(
        buffer,
        max,
        "MemTotal: %u KB\nMemUsed: %u KB\n",
        kernel_total_memory / 1024,
        kernel_used_memory / 1024
    );
}

static size_t proc_iomem_read(char *buf, size_t max) {
    return iomem_print(buf, max);
}

static fs_node_t file_meminfo = {
    .name = "meminfo",
    .type = FS_FILE,
    .read = proc_meminfo_read
};

static fs_node_t file_iomem = {
    .name = "iomem",
    .type = FS_FILE,
    .read = proc_iomem_read
};

static fs_node_t *proc_children[] = {
    &file_meminfo,
    &file_iomem
};

fs_node_t dir_proc = {
    .name = "proc",
    .type = FS_DIR,
    .children = proc_children,
    .child_count = 2
};