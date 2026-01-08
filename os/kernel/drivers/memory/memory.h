#pragma once
#include <stddef.h>
#include <arch.h>

extern char kernel_start;
extern char kernel_end;

extern size_t kernel_total_memory;
extern size_t kernel_used_memory;
extern size_t kernel_mmio_memory;
void memory_init(struct multiboot_info *mbi);