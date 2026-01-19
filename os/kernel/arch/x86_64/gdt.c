#include "gdt.h"

static uint64_t gdt[3];
static gdt_ptr_t gdt_ptr;

extern void gdt_flush(gdt_ptr_t *);

void gdt_init(void) {
    gdt[0] = 0;

    gdt[1] =
        (1ULL << 44) |
        (1ULL << 47) |
        (1ULL << 41) |
        (1ULL << 43);

    gdt[2] =
        (1ULL << 44) |
        (1ULL << 47) |
        (1ULL << 41);

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    gdt_flush(&gdt_ptr);
}
