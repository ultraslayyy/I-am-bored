#include "iomem.h"
#include "memory.h"
#include <lib/string.h>

#define MAX_IOMEM 32

static iomem_region_t regions[MAX_IOMEM];
static size_t region_count = 0;

extern char text_start, text_end;
extern char rodata_start, rodata_end;
extern char data_start, data_end;
extern char bss_start, bss_end;

static void iomem_add(uint64_t start, uint64_t end, const char *name, int indent) {
    if (region_count >= MAX_IOMEM)
        return;

    regions[region_count++] = (iomem_region_t){
        .start = start,
        .end = end,
        .name = name,
        .indent = indent
    };
}

void iomem_init(void) {
    iomem_add(0x00000000, 0x0009FFFF, "System RAM", 0);
    iomem_add(0x000A0000, 0x000BFFFF, "Reserved", 0);

    iomem_add(0x000B8000, 0x000B8FA0, "VGA text buffer", 1);

    iomem_add(0x000C0000, 0x000FFFFF, "System ROM", 0);

    iomem_add(
        (uint64_t)&kernel_start,
        (uint64_t)&kernel_end - 1,
        "Kernel image",
        0
    );

    iomem_add(
        (uint64_t)&text_start,
        (uint64_t)&text_start + 0x20000,
        "Kernel code",
        1
    );

    iomem_add(
        (uint64_t)&rodata_start,
        (uint64_t)&rodata_end - 1,
        "Kernel RO Data",
        1
    );

    iomem_add(
        (uint64_t)&data_start,
        (uint64_t)&data_end - 1,
        "Kernel Data",
        1
    );

    iomem_add(
        (uint64_t)&bss_start,
        (uint64_t)&bss_end - 1,
        "Kernel BSS",
        1
    );
}

size_t iomem_print(char *buf, size_t max) {
    size_t written = 0;

    for (size_t i = 0; i < region_count; i++) {
        const iomem_region_t *r = &regions[i];

        for (int j = 0; j < r->indent; j++) {
            written += snprintf(buf + written, max - written, "  ");
        }

        written += snprintf(
            buf + written,
            max - written,
            "%x-%x : %s\n",
            (uint32_t)r->start,
            (uint32_t)r->end,
            r->name
        );
    }

    return written;
}