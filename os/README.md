# UltraOS

## Dependencies
First, install dependencies

### Ubuntu/Debian based
```bash
sudo apt update
sudo apt install build-essential nasm grub-pc-bin xorriso mtools qemu-system-i386 qemu-system-x86_64 dosfstools
```

### Arch based
```sh
sudo pacman -S base-devel nasm grub xorriso mtools qemu gdb dosfstools
```

## Building
You build using `make`.
> ![IMPORTANT]
> The x86_64 architecture currently does not work and is unsupported.

By default, `make` will build `x86`, but you can also specify an arch
```bash
make ARCH=arch
```
Current arches are:
- x86
- x86_64

## Running

Before running, you should make a FAT16 disk image to mount (for full functionality). I am currently working on something that will default to ramfs if the image is not found.
```sh
dd if=/dev/zero of=disk.img bs=1M count=16
mkfs.fat -F 16 disk.img

# Optionally put some files in it
sudo mkdir /mnt/ultraos
sudo mount -o loop disk.img /mnt/ultraos
sudo cp file /mnt/ultraos
sudo umount /mnt/temp
```

Then you can run it:
```sh
make run
```

## Roadmap

drivers:
- USB support (just start by listing USB devices)
- Maybe HDMI support (just listing connected displays, not using them till much later)
- RTL8169 support (check net section)
- Stub for unimplemented drivers

fs:
- fat12
- fat32
- Actual mounting
- Detect FS on boot and init that FS
- Work outside of QEMU
- Graceful handling of unknown FS

net:
- Support multiple network devices at once, with one being 'current' which all ops happen on
- Single network device initialisation function for initialising all network devices (if present)
- RTL8169 support
- Loopback interface

Power management/`shutdown`:
- Actually shut down computer (work outside QEMU):
  - Find RSDP
  - Parse RSDT/XSDT
  - Locate FADT
  - Get PM1a and PM1b control block addresses from DSDT
  - Get S5 sleep type values from DSDT
  - Write PM1 sleep commands to PM1 control register
  - Test in VM before real hardware
- "shutdown not supported" message for unsupported devices
- `reboot` and `sleep` functionality
- Battery status and ACPI power events
  - For laptops

<!--

Creating user programs (future reference)

```sh
# Compile user libraries
i686-elf-gcc -ffreestanding -c string.c stdlib.c -o string.o stdlib.o

# Compile user program
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -c userprog.c -o userprog.o

# Link into ELF for OS
i686-elf-ld -T user_link.ld userprog.o string.o stdlib.o -o userprog.elf
```

-->