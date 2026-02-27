#include <arch.h>
#include "memory.h"
#include "iomem.h"

size_t kernel_total_memory = 0;
size_t kernel_used_memory = 0;

void memory_init(boot_info_t *mbi) {
    kernel_used_memory = (size_t)&kernel_end - (size_t)&kernel_start;

    if (mbi->mmap_entries == 0) {
        kernel_total_memory = 0;
        return;
    }

    for (uint32_t i = 0; i < mbi->mmap_entries; ++i) {
        boot_mmap_entry_t *entry = &mbi->mmap[i];

        if (entry->type == 1) {
            kernel_total_memory += entry->length;
        }
    }

    // Causes triple fault at ln 17
    // iomem_init(mbi);
}
