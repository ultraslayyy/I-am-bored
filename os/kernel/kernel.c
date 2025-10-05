void kernel_main() {
    const char* msg = "MUM ARE YOU PROUD OF ME!?";
    char* vidmem = (char*) 0xb8000;

    for (int i = 0; msg[i] != '\0'; i++) {
        vidmem[i * 2] = msg[i];
        vidmem[i * 2 + 1] = 0x07;
    }

    while (1) {}
}
