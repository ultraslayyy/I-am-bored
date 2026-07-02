# Toolchain
CC := gcc
LD := ld
AS := nasm

# Architecture selection
ARCH ?= x86

# GDB
GDB ?= 0

# Architecture-specific flags
ifeq ($(ARCH),x86)
  CFLAGS_ARCH   := -m32
  LDFLAGS_ARCH  := -m elf_i386
  NASM_FMT      := elf32
  QEMU_BIN      := qemu-system-i386
  LINKER_SCRIPT := kernel/arch/x86/linker.ld
  BUILD_DIR     := build/x86
  KERNEL_ELF    := $(BUILD_DIR)/kernel_x86.elf
else ifeq ($(ARCH),x86_64)
  CFLAGS_ARCH  := -m64 -mno-red-zone
  LDFLAGS_ARCH := -m elf_x86_64
  NASM_FMT     := elf64
  QEMU_BIN     := qemu-system-x86_64
  LINKER_SCRIPT:= kernel/arch/x86_64/linker.ld
  BUILD_DIR     := build/x86_64
  KERNEL_ELF   := $(BUILD_DIR)/kernel_x86_64.elf
else
  $(error Unsupported ARCH=$(ARCH))
endif

# General flags
CFLAGS := $(CFLAGS_ARCH) -ffreestanding -fno-stack-protector -fno-pic \
          -nostdlib -nostdinc -Wall -Wextra -Wshadow -ffunction-sections \
          -fdata-sections -Ikernel -Iinclude
LDFLAGS := $(LDFLAGS_ARCH) -T $(LINKER_SCRIPT) --gc-sections

ISO        := ultraos.iso
ISO_DIR    := iso

# Source files
C_SOURCES   := $(filter-out kernel/usr/%,$(shell find kernel -name '*.c'))
ASM_SOURCES := $(shell find kernel -name '*.s')

# Separate arch-specific files
X86_C_SRC    := $(shell find kernel/arch/x86 -name '*.c')
X86_ASM_SRC  := $(shell find kernel/arch/x86 -name '*.s')
X86_64_C_SRC := $(shell find kernel/arch/x86_64 -name '*.c')
X86_64_ASM_SRC := $(shell find kernel/arch/x86_64 -name '*.s')

CORE_C_SRC   := $(filter-out $(X86_C_SRC) $(X86_64_C_SRC), $(C_SOURCES))
CORE_ASM_SRC := $(filter-out $(X86_ASM_SRC) $(X86_64_ASM_SRC), $(ASM_SOURCES))

# === Objects ===
CORE_C_OBJ   := $(patsubst kernel/%.c,$(BUILD_DIR)/%.o,$(CORE_C_SRC))
CORE_ASM_OBJ := $(patsubst kernel/%.s,$(BUILD_DIR)/%.o,$(CORE_ASM_SRC))

X86_C_OBJ    := $(patsubst kernel/%.c,$(BUILD_DIR)/%.o,$(X86_C_SRC))
X86_ASM_OBJ  := $(patsubst kernel/%.s,$(BUILD_DIR)/%.o,$(X86_ASM_SRC))

X86_64_C_OBJ   := $(patsubst kernel/%.c,$(BUILD_DIR)/%.o,$(X86_64_C_SRC))
X86_64_ASM_OBJ := $(patsubst kernel/%.s,$(BUILD_DIR)/%.o,$(X86_64_ASM_SRC))

ifeq ($(ARCH),x86)
  OBJECTS := $(CORE_C_OBJ) $(CORE_ASM_OBJ) $(X86_C_OBJ) $(X86_ASM_OBJ)
else
  OBJECTS := $(CORE_C_OBJ) $(CORE_ASM_OBJ) $(X86_64_C_OBJ) $(X86_64_ASM_OBJ)
endif

QEMUFLAGS := -m 512 -serial stdio -no-reboot -drive file=disk.img,format=raw -device rtl8139,netdev=net0 -netdev user,id=net0 \
             -object filter-dump,id=f1,netdev=net0,file=packets.pcap -serial file:serial.log # -d int,guest_errors -D qemu.log # -device usb-ehci,id=ehci -device usb-kbd,bus=ehci.0
# Can do wireshark with packets.pcap
# Do multiple NICs by adding -device <e1000|rtl8139>,netdev=net1 -netdev user,id=net1 (increase the netX number for each card)
# Would need to still capture each individually on wireshark, so a duplicate of -object is required. For example:
# -object filter-dump,id=f0,netdev=net0,file=e1000.pcap -object filter-dump,id=f1,netdev=net1,file=rtl8139.pcap

ifeq ($(GDB),1)
  CFLAGS += -O0 -g
  QEMUFLAGS += -S -s
else
  CFLAGS += -O2
endif

# Targets
all: $(ISO)

$(ISO): $(KERNEL_ELF)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS_ARCH) -T $(LINKER_SCRIPT) --gc-sections -o $@ $^

# Build
$(BUILD_DIR)/%.o: kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: kernel/%.s
	mkdir -p $(dir $@)
	$(AS) -f $(NASM_FMT) $< -o $@

run: $(ISO)
	$(QEMU_BIN) -boot d -cdrom $(ISO) $(QEMUFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) *.iso

.PHONY: all run clean
