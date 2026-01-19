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

static void iomem_add(uintptr_t start, uintptr_t end, const char *name, int indent) {
    if (region_count >= MAX_IOMEM) return;
    regions[region_count++] = (iomem_region_t){start, end, name, indent};
}

static void iomem_sort(void) {
    for (size_t i = 0; i + 1 < region_count; i++) {
        for (size_t j = 0; j + 1 < region_count - i; j++) {
            if (regions[j].start > regions[j + 1].start) {
                iomem_region_t tmp = regions[j];
                regions[j] = regions[j + 1];
                regions[j + 1] = tmp;
            }
        }
    }
}

static void iomem_merge(void) {
    if (region_count == 0) return;
    iomem_sort();

    size_t dst = 0;
    for (size_t i = 1; i < region_count; i++) {
        iomem_region_t *prev = &regions[dst];
        iomem_region_t *curr = &regions[i];

        if (prev->end >= curr->start && prev->indent == curr->indent &&
            strcmp(prev->name, curr->name) == 0) {
            if (curr->end > prev->end) prev->end = curr->end;
        } else {
            dst++;
            if (dst != i) regions[dst] = regions[i];
        }
    }

    region_count = dst + 1;
}

void iomem_init(boot_info_t *mbi) {
    if (mbi && mbi->mmap_entries != 0) {
        for (uint32_t i = 0; i < mbi->mmap_entries; i++) {
            boot_mmap_entry_t *entry = &mbi->mmap[i];

            if (entry->type != 1) {
                iomem_add(entry->base,
                        entry->base + entry->length,
                        "Reserved", 0);
            } else {
                iomem_add(entry->base,
                        entry->base + entry->length,
                        "System RAM", 0);
            }
        }
    }

    iomem_add(0x000B8000, 0x000B8FA0, "VGA text buffer", 1);
    iomem_add(0x000A0000, 0x000BFFFF, "Reserved", 0);
    iomem_add(0x000C0000, 0x000FFFFF, "System ROM", 0);

    iomem_add(
        (uintptr_t)&kernel_start,
        (uintptr_t)&kernel_end,
        "Kernel image",
        0
    );

    iomem_add(
        (uintptr_t)&text_start,
        (uintptr_t)&text_start,
        "Kernel code",
        1
    );

    iomem_add(
        (uintptr_t)&rodata_start,
        (uintptr_t)&rodata_end,
        "Kernel RO Data",
        1
    );

    iomem_add(
        (uintptr_t)&data_start,
        (uintptr_t)&data_end,
        "Kernel Data",
        1
    );

    iomem_add(
        (uintptr_t)&bss_start,
        (uintptr_t)&bss_end,
        "Kernel BSS",
        1
    );

    iomem_merge();
}

size_t iomem_print(char *buf, size_t max) {
    size_t written = 0;

    for (size_t i = 0; i < region_count; i++) {
        const iomem_region_t *r = &regions[i];

        for (int j = 0; j < r->indent; j++) {
            if (written + 2 < max) {
                buf[written++] = ' ';
                buf[written++] = ' ';
            }
        }

        int n = snprintf(
            buf + written,
            max > written ? max - written : 0,
            "0x%x-0x%x : %s\n",
            (unsigned int)r->start,
            (unsigned int)r->end,
            r->name
        );
        if (n < 0) break;
        if ((size_t)n >= max - written) {
            written = max;
            break;
        }
        written += n;
    }

    return written;
}