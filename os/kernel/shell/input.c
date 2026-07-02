#include <io/kernel_io.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include <io/keyboard.h>
#include "shell.h"

void handle_input_char(char c, char *input_buffer, size_t *input_pos) {
    if (c == '\b') {
        if (*input_pos > 0) {
            (*input_pos)--;
            put_char('\b', 0);
        }
        return;
    }

    if (c == '\n') {
        input_buffer[*input_pos] = 0;
        (*input_pos) = 0;

        put_char('\n', 0);
        process_command(input_buffer);

        char path[MAX_PATH_LEN];
        vfs_get_path(kernel_cwd, path, sizeof(path));
        char prompt[MAX_PATH_LEN + MAX_INPUT];
        snprintf(prompt, sizeof(prompt), "%s $ ", path);

        put_string(prompt, 0);
        return;
    }

    if (*input_pos < MAX_INPUT - 1) {
        input_buffer[(*input_pos)++] = c;
        put_char(c, 0);
    }
}

static char s_input_buffer[MAX_INPUT];
static size_t s_input_pos = 0;

void shell_handle_char(char c) {
    handle_input_char(c, s_input_buffer, &s_input_pos);
}

void update_shell_input(void) {
    key_event_t ev;
    while (keyboard_pop_event(&ev)) {
        if (ev.pressed) {
            char c = keycode_to_char(ev.scancode, shift_pressed);
            if (c) {
                shell_handle_char(c);
            }
        }
    }
}
