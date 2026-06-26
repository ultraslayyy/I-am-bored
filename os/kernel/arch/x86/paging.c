#include "paging.h"

#define PAGE_ENTRIES 1024

static uint32_t page_directory[PAGE_ENTRIES]  __attribute__((aligned(4096)));

static uint32_t ident_tables[4][PAGE_ENTRIES] __attribute__((aligned(4096)));
static uint32_t heap_page_table[PAGE_ENTRIES] __attribute__((aligned(4096)));

void paging_init(void) {
    // Identity map first 16 MB
    for (uint32_t i = 0; i < 4; ++i) {
        for (uint32_t j = 0; j < PAGE_ENTRIES; ++j) {
            uint32_t addr = (i * 0x400000) + (j * PAGE_SIZE);
            ident_tables[i][j] = addr | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        page_directory[i] = ((uint32_t)ident_tables[i]) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    // Clear remaining entries
    for (uint32_t i = 4; i < PAGE_ENTRIES; ++i) {
        page_directory[i] = 0;
    }

    // Map the heap (16MB to 20MB, 1MB heap)
    for (uint32_t i = 0; i < PAGE_ENTRIES; ++i) {
        uint32_t addr = (0x01000000 + i * PAGE_SIZE);
        heap_page_table[i] = addr | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }
    page_directory[4] = ((uint32_t)&heap_page_table) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

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
            page_directory[dir_idx] |= (PAGE_USER | PAGE_RW);
        }
    } else {
        // Allocate a new page table at a safe location
        table = (uint32_t *)(0x200000 + dir_idx * PAGE_SIZE);
        for (int i = 0; i < 1024; ++i) {
            table[i] = 0;
        }
        
        uint32_t pd_flags = PAGE_PRESENT | PAGE_RW;
        if (flags & PAGE_USER) {
            pd_flags |= PAGE_USER;
        }

        page_directory[dir_idx] = ((uint32_t)table) | pd_flags;
    }

    if (table[table_idx] & PAGE_PRESENT) {
        return;
    }

    table[table_idx] = (phys & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;
}