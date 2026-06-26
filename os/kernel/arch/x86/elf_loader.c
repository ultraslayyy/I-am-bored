#include <lib/string.h>
#include "elf_loader.h"
#include "paging.h"
#include "user.h"

#define USER_PROGRAM_STACK 0x500000
#define PAGE_FLAGS (PAGE_PRESENT | PAGE_RW | PAGE_USER)

typedef struct {
    uint8_t  ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
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

    if (eh->ident[0] != 0x7F || eh->ident[1] != 'E' || eh->ident[2] != 'L' || eh->ident[3] != 'F') {
        return;
    }

    if (eh->ident[4] != 1) return; // e_ident[EI_CLASS] = 1 means 32-bit, 2 is 64-bit

    if (eh->phnum > 32) return;
    if (eh->phoff > 0x100000) return;

    for (uint16_t i = 0; i < eh->phnum; ++i) {
        program_header_t *ph = (program_header_t *)(elf_data + eh->phoff + i * eh->phentsize);

        if (ph->type != 1) continue;

        uint32_t pages = (ph->memsz + 0xFFF) / PAGE_SIZE;
        for (uint32_t p = 0; p < pages; ++p) {
            map_page(ph->vaddr + p * PAGE_SIZE, ph->vaddr + p * PAGE_SIZE, PAGE_FLAGS);
        }

        memcpy((void *)ph->vaddr, elf_data + ph->offset, ph->filesz);
        if (ph->memsz > ph->filesz) {
            memset((uint8_t *)(ph->vaddr) + ph->filesz, 0, ph->memsz - ph->filesz);
        }
    }

    for (int i = 0; i < 4; ++i) {
        map_page(USER_PROGRAM_STACK - i * PAGE_SIZE, USER_PROGRAM_STACK - i * PAGE_SIZE, PAGE_FLAGS);
    }

    if (eh->entry < 0x400000 || eh->entry >= 0x800000) {
        return;
    }
    
    enter_user_mode((void (*)())eh->entry);
}