ENTRY(_start)

SECTIONS {
  . = 0x00100000;

  kernel_start = .;

  .multiboot : { KEEP(*(.multiboot)) }
  .text : ALIGN(4K) {
    text_start = .;
    *(.text*)
    text_end = .;
  }
  .rodata : ALIGN(4K) {
    rodata_start = .;
    *(.rodata*)
    rodata_end = .;
  }
  .data : ALIGN(4K) {
    data_start = .;
    *(.data*)
    data_end = .;
  }
  .bss : ALIGN(4K) {
    bss_start = .;
    *(.bss*)
    *(COMMON)
    bss_end = .;
  }
  .stack (NOLOAD) : ALIGN(16) {
    . = ALIGN(16);
    kernel_stack_start = .;
    . = . + 0x2000; /* 8 KB */
    kernel_stack_end = .;
  }

  kernel_end = .;
}
