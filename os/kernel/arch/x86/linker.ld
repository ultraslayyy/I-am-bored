ENTRY(_start)

SECTIONS {
  . = 0x00100000;

  kernel_start = .;

  .multiboot : { KEEP(*(.multiboot)) }
  .text : {
    text_start = .;
    *(.text*)
    text_end = .;
  }
  .rodata : {
    rodata_start = .;
    *(.rodata*)
    rodata_end = .;
  }
  .data : {
    data_start = .;
    *(.data*)
    data_end = .;
  }
  .bss : {
    bss_start = .;
    *(.bss*)
    *(COMMON)
    bss_end = .;
  }
  kernel_stack_start = .;
  .stack : {
    . = . + 0x2000;
  }
  kernel_stack_end = .;

  kernel_end = .;
}
