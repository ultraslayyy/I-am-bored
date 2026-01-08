#include <stdint.h>
#include <stddef.h>
#include <io/kernel_io.h>
#include <io/keyboard.h>
#include <shell/shell.h>
#include <shell/input.h>
#include <fs/fs.h>
#include <drivers/memory/memory.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void kernel_main() {
    clear_screen();

    char input_buffer[MAX_INPUT];
    size_t input_pos = 0;

    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    uint8_t shift_pressed = 0;
    uint8_t key_pressed[128] = {0};

    memory_init();
    fs_init();
    shell_init();

    while (1) {
        uint8_t sc = inb(0x60);
        if (sc == 0) continue;

        if (sc & 0x80) {
            uint8_t released = sc & 0x7F;
            key_pressed[released] = 0;
            handle_key_release(released, &shift_pressed);
            continue;
        }

        if (key_pressed[sc]) continue;
        key_pressed[sc] = 1;

        if (sc == 0x2A || sc == 0x36) {
            shift_pressed = 1;
            continue;
        }
        
        if (sc == 0x48) {
            scroll_up();
            continue;
        } else if (sc == 0x50) {
            scroll_down();
            continue;
        }

        char c = keycode_to_char(sc, shift_pressed);
        if (!c) continue;

        handle_input_char(c, input_buffer, &input_pos, prompt);
    }
}
