#include "memory.h"
#include "iomem.h"
#include <arch.h>

size_t kernel_total_memory = 0;
size_t kernel_used_memory = 0;

void memory_init(struct multiboot_info *mbi) {
    kernel_used_memory = (size_t)&kernel_end - (size_t)&kernel_start;

    if (!(mbi->flags & (1 << 6))) {
        kernel_total_memory = 0;
        return;
    }

    uintptr_t mmap_end = (uintptr_t)mbi->mmap_addr + mbi->mmap_length;
    uintptr_t mmap_ptr = (uintptr_t)mbi->mmap_addr;
    
    while (mmap_ptr < mmap_end) {
        multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t *)mmap_ptr;
        if (entry->type == 1) {
            kernel_total_memory += entry->len;
        }

        mmap_ptr += entry->size + sizeof(entry->size);
    }

    iomem_init(mbi);
}
