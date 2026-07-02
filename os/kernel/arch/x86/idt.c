#include <drivers/mouse/ps2.h>
#include <io/kernel_io.h>
#include "idt.h"
#include "io.h"

extern void irq0(void);
extern void irq1(void);
extern void irq12(void);
extern uint32_t schedule(uint32_t esp);

extern void keyboard_callback(void);

idt_entry_t idt[IDT_ENTRIES];
idtr_t idtr;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].isr_low = base & 0xFFFF;
    idt[num].kernel_cs = sel;
    idt[num].reserved = 0;
    idt[num].attributes = flags;
    idt[num].isr_high = (base >> 16) & 0xFFFF;
}

static void pic_remap(void) {
    // Master PIC = 0x20, Slave PIC = 0xA0
    // ICW1
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();

    // ICW2 (IDT offset)
    outb(0x21, 0x20); // Master -> 0x20 (32)
    io_wait();
    outb(0xA1, 0x28); // Slave -> 0x28 (40)
    io_wait();

    // ICW3
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();

    // ICW4
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    // Mask all except IRQ 0 (Timer), IRQ 1 (Keyboard), and IRQ2 (cascade from slave triggering)
    outb(0x21, 0xF8); // 1111 1000
    // Mask all except IRQ12 (mouse on PS/2)
    outb(0xA1, 0xEF); // 1110 1111 
}

void idt_init(void) {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint32_t)&idt;

    // Clear IDT
    for (size_t i = 0; i < IDT_ENTRIES; ++i) {
        idt[i].isr_low = 0;
        idt[i].kernel_cs = 0;
        idt[i].reserved = 0;
        idt[i].attributes = 0;
        idt[i].isr_high = 0;
    }

    pic_remap();

    // Install IRQ 0 (Timer) at 32 (0x20)
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E); // 0x08 is kernel code segment, 0x8E is 32-bit intr gate
    // Install IRQ 1 (Keyboard) at 33 (0x21)
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E); // 0x08 is kernel code segment, 0x8E is 32-bit intr gate
    // Install IRQ 12 (mouse via PS/2) at 44 (0x2C)
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E); // 0x08 is kernel code segment, 0x8E is 32-bit intr gate

    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("sti");
}

void *isr_handler(uint32_t int_num, uint32_t esp) {
    if (int_num == 32) {
        esp = schedule(esp);
    }
    
    if (int_num == 33) {
        keyboard_callback();
    }

    if (int_num == 44) {
        mouse_handler();
    }

    // Send EOI to PIC
    if (int_num >= 32) {
        outb(0x20, 0x20); // EOI to Master
        if (int_num >= 40) {
            outb(0xA0, 0x20); // EOI to Slave
        }
    }

    return (void *)esp;
}
