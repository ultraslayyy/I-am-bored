#include <lib/string.h>
#include "gdt.h"
#include "tss.h"

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;

    uint32_t unused[23];
} __attribute__((packed)) tss_entry_t;

static tss_entry_t tss;

void tss_set_stack(uint32_t stack) {
    tss.esp0 = stack;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    gdt_set_gate(num, base, limit, 0x89, 0x40); // Present, ring 0, type 9 (32-bit TSS)

    memset(&tss, 0, sizeof(tss));

    tss.ss0 = ss0;
    tss.esp0 = esp0;
}

void tss_init(uint32_t kernel_stack) {
    write_tss(5, 0x10, kernel_stack); // GDT entry 5
    asm volatile("ltr %%ax" :: "a"(5 << 3));
}