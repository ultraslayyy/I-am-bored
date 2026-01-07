CC := gcc
LD := ld
AC := nasm

ARCH := -m32

CFLAGS  := $(ARCH) -ffreestanding -nostdlib -Wall -Wextra -ffunction-sections -fdata-sections -Ikernel
LDFLAGS := -m elf_i386 -T kernel/arch/x86/linker.ld --gc-sections

ISO        := ultraos.iso
ISO_DIR    := iso
BUILD_DIR  := build
KERNEL_ELF := $(BUILD_DIR)/kernel.elf

C_SOURCES   := $(shell find kernel -name '*.c')
ASM_SOURCES := $(shell find kernel -name '*.s')

C_OBJECTS   := $(patsubst kernel/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst kernel/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
OBJECTS     := $(C_OBJECTS) $(ASM_OBJECTS)

all: $(ISO)

$(ISO): $(KERNEL_ELF)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ build/arch/x86/boot/boot.o $(filter-out build/arch/x86/boot/boot.o,$(OBJECTS))

$(BUILD_DIR)/%.o: kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: kernel/%.s
	mkdir -p $(dir $@)
	$(AC) -f elf32 $< -o $@

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 512 -net none

run-fs: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 512 -net none -vga std -full-screen -display sdl

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) *.iso

.PHONY: all run clean