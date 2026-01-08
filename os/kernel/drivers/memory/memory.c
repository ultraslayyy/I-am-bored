#include "memory.h"
#include "iomem.h"

size_t kernel_total_memory = 32 * 1024 * 1024;
size_t kernel_used_memory = 0;

void memory_init(void) {
    kernel_used_memory = (size_t)&kernel_end - (size_t)&kernel_start;
    iomem_init();
}
