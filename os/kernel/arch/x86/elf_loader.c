#include "elf_loader.h"
#include "paging.h"
#include "user.h"

#define USER_PROGRAM_VADDR 0x400000
#define USER_PROGRAM_STACK 0x500000
#define PAGE_FLAGS (PAGE_PRESENT | PAGE_RW | PAGE_USER)

typedef struct {
    uint32_t magic;
    uint8_t  elf[12];
    uint16_t type;
    uint16_t machine;
    uint16_t version;
    uint16_t entry;
    uint16_t phoff;
    uint16_t shoff;
    uint16_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} __attribute__((packed)) program_header_t;

void load_elf(uint8_t *elf_data) {
    elf_header_t *eh = (elf_header_t *)elf_data;

    // Iterate over program headers
    for (uint16_t i = 0; i < eh->phnum; ++i) {
        program_header_t *ph = (program_header_t *)(elf_data + eh->phoff + i * eh->phentsize);

        if (ph->type != 1) continue; // Only loadable segments

        uint32_t pages = (ph->memsz + 0xFFF) / 0x1000;
        for (uint32_t p = 0; p < pages; ++p) {
            map_page(ph->vaddr + p * 0x1000, ph->paddr + p * 0x1000, PAGE_FLAGS);
        }

        // Copy segment data
        for (uint32_t b = 0; b < ph->filesz; ++b) {
            ((uint8_t *)ph->vaddr)[b] = elf_data[ph->offset + b];
        }

        // Zero out remaining bytes in memory
        for (uint32_t b = ph->filesz; b < ph->memsz; ++b) {
            ((uint8_t *)ph->vaddr)[b] = 0;
        }
    }

    // Map user stack
    map_page(USER_PROGRAM_STACK, USER_PROGRAM_STACK, PAGE_FLAGS);

    // Jump to entry
    enter_user_mode((void (*)())eh->entry);
}