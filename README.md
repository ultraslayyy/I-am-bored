# UltraOS

## Installing and running locally
> This only runs on Ubuntu, Debian, or WSL.

First, install dependencies

```bash
sudo apt update
sudo apt install build-essential nasm grub-pc-bin xorriso mtools qemu-system-i386
```

Then build and run the ISO

```bash
make           # Build the ISO
make run       # Boot it in QEMU
```