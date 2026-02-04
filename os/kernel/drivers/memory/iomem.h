#pragma once
#include <lib/stdint.h>
#include <core/boot_info.h>
#include <lib/stddef.h>
#include <arch.h>

typedef struct iomem_region {
    uint64_t start;
    uint64_t end;
    const char *name;
    size_t indent;
} iomem_region_t;

void iomem_init(boot_info_t *mbi);
size_t iomem_print(char *buf, size_t max);