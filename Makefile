CC = gcc
LD = ld

ISO = ultraos.iso
KERNEL_ELF = kernel/kernel.elf

all: $(ISO)

$(ISO): $(KERNEL_ELF)
	mkdir -p iso/boot/grub
	cp $(KERNEL_ELF) iso/boot/kernel.elf
	cp grub/grub.cfg iso/boot/grub/
	grub-mkrescue -o $(ISO) iso/

kernel/kernel.o: kernel/kernel.c
	$(CC) -m32 -ffreestanding -nostdlib -c $< -o $@

kernel/boot.o: kernel/boot.s
	nasm -f elf32 $< -o $@

$(KERNEL_ELF): kernel/boot.o kernel/kernel.o kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $@ kernel/boot.o kernel/kernel.o

run: $(ISO)
	qemu-system-i386 -boot d -cdrom $(ISO) -m 512

clean:
	rm -rf kernel/*.o kernel/*.elf iso *.iso
