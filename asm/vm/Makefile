ASM       := nasm
ASMFLAGS  := -f elf64 -F dwarf # -g
LD        := ld
LDFLAGS   := # -g

TARGET    := vm

SRC_DIR   := src
BUILD_DIR := build

SRCS      := $(shell find $(SRC_DIR) -name '*.asm')

OBJS      := $(SRCS:$(SRC_DIR)/%.asm=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking executable: $(TARGET)"
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "Assembling: $<"
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean