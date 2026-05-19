#include "paging.h"

#define PAGE_ENTRIES 1024

static uint32_t page_directory[PAGE_ENTRIES]   __attribute__((aligned(4096)));
static uint32_t first_page_table[PAGE_ENTRIES] __attribute__((aligned(4096)));

void paging_init(void) {
    (void)first_page_table;

    // Identity map first 8 MB
    for (uint32_t i = 0; i < 2; ++i) {
        static uint32_t ident_tables[2][PAGE_ENTRIES] __attribute__((aligned(4096)));
        for (uint32_t j = 0; j < PAGE_ENTRIES; ++j) {
            ident_tables[i][j] = ((i * 0x400000) + (j * 0x1000)) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        page_directory[i] = ((uint32_t)ident_tables[i]) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    // Clear other entries (starting from index 2)
    for (uint32_t i = 2; i < PAGE_ENTRIES; ++i) {
        if (i == 4) continue; // Skip heap entry, handled below
        page_directory[i] = 0;
    }

    // Map the heap (16MB to 17MB)
    // Heap starts at 0x01000000 (index 4)
    // Second pre-allocated table for simplicity
    static uint32_t heap_page_table[PAGE_ENTRIES] __attribute__((aligned(4096)));
    for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
        heap_page_table[i] = (0x01000000 + i * 0x1000) | PAGE_PRESENT | PAGE_RW;
    }
    page_directory[4] = ((uint32_t)heap_page_table) | PAGE_PRESENT | PAGE_RW;

    // Load the page directory into CR3
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging (set PG bit in CR0)
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

// Helper to map any virtual to physical page
void map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_idx = virt >> 22;
    uint32_t table_idx = (virt >> 12) & 0x3FF;

    uint32_t *table;
    if (page_directory[dir_idx] & PAGE_PRESENT) {
        table = (uint32_t *)(page_directory[dir_idx] & 0xFFFFF000);
        if (flags & PAGE_USER) {
            page_directory[dir_idx] |= PAGE_USER;
        }
    } else {
        // Allocate a new page table at a safe location
        table = (uint32_t *)(0x200000 + dir_idx * 0x1000);
        for (int i = 0; i < 1024; ++i) {
            table[i] = 0;
        }
        
        uint32_t pd_flags = PAGE_PRESENT | PAGE_RW;
        if (flags & PAGE_USER) pd_flags |= PAGE_USER;

        page_directory[dir_idx] = ((uint32_t)table) | pd_flags;
    }

    table[table_idx] = (phys & 0xFFFFF000) | (flags & 0xFFF);
}