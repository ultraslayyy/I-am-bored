# UltraOS

## Dependencies
First, install dependencies

### Ubuntu/Debian based
```bash
sudo apt update
sudo apt install build-essential nasm grub-pc-bin xorriso mtools qemu-system-i386 qemu-system-x86_64
```

### Arch based
```sh
sudo pacman -S base-devel nasm grub xorriso mtools qemu gdb
```

## Building
You build using `make`.
> ![IMPORTANT]
> The x86_64 architecture currently builds but does not run.

By default, `make` will build `x86`, but you can also specify an arch
```bash
make ARCH=arch
```
Current arches are:
- x86
- x86_64

You can run with `make run`