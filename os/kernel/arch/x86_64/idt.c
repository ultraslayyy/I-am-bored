#include <arch/x86/io.h>
#include <lib/stddef.h>
#include "idt.h"

extern void irq1(void);

static idt_entry_t idt[IDT_ENTRIES];
static idtr_t idtr;

static void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].selector    = sel;
    idt[num].ist         = 0;
    idt[num].attributes  = flags;
    idt[num].offset_mid  = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

void idt_init(void) {
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;

    for (size_t i = 0; i < IDT_ENTRIES; ++i) {
        idt_set_gate(i, 0, 0, 0);
    }
    idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);

    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("sti");
}

void isr_handler(uint64_t int_num) {
    if (int_num == 33) {
        keyboard_callback();
    }

    outb(0x20, 0x20);
}
