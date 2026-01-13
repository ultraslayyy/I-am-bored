#pragma once
#include <lib/stdint.h>
#include <lib/stddef.h>
#include <arch.h>

typedef struct iomem_region {
    uint64_t start;
    uint64_t end;
    const char *name;
    int indent;
} iomem_region_t;

void iomem_init(struct multiboot_info *mbi);
size_t iomem_print(char *buf, size_t max);