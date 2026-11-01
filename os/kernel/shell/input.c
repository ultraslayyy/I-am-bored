#include <io/kernel_io.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include <io/keyboard.h>
#include "history.h"
#include "shell.h"

char shell_saved_input[MAX_INPUT] = "";

void handle_input_char(char c, char *input_buffer, size_t *input_pos) {
    if (c == '\b') {
        if (*input_pos > 0) {
            if (ctrl_pressed) {
                do {
                    put_char('\b', 0);
                } while (input_buffer[--(*input_pos)] != ' ' && *input_pos > 0);
            } else {
                (*input_pos)--;
                put_char('\b', 0);
            }
        }
        return;
    }

    if (c == '\n') {
        input_buffer[*input_pos] = 0;
        (*input_pos) = 0;

        put_char('\n', 0);
        process_command(input_buffer);

        history_reset_navigation();

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

void history_save_input(const char *input) {
    strlcpy(shell_saved_input, input, MAX_INPUT);
}

void shell_handle_char(char c) {
    handle_input_char(c, s_input_buffer, &s_input_pos);
}

// TODO: Make it handle wrapping properly
void shell_set_input(const char *str) {
    for (size_t i = 0; i < s_input_pos; ++i) {
        put_char('\b', DEFAULT_ATTR);
        put_char(' ', DEFAULT_ATTR);
        put_char('\b', DEFAULT_ATTR);
    }

    strlcpy(s_input_buffer, str, MAX_INPUT);
    s_input_pos = strlen(s_input_buffer);

    put_string(s_input_buffer, DEFAULT_ATTR);
}

void update_shell_input(void) {
    key_event_t ev;
    while (keyboard_pop_event(&ev)) {
        if (!ev.pressed) continue;

        switch (ev.scancode) {
            case 0x48: {
                if (shift_pressed) {
                    scroll_up();
                    break;
                }
                if (!history_is_navigating()) {
                    history_save_input(s_input_buffer);
                }
                const char *cmd = history_previous();
                if (cmd) {
                    shell_set_input(cmd);
                }
                break;
            }
            case 0x50: {
                if (shift_pressed) {
                    scroll_down();
                    break;
                }
                if (!history_is_navigating()) {
                    history_save_input(s_input_buffer);
                }
                const char *cmd = history_next();
                if (cmd) {
                    shell_set_input(cmd);
                }
                break;
            }
            default: {
                char c = keycode_to_char(ev.scancode, shift_pressed);
                if (c) {
                    shell_handle_char(c);
                }
            }
        }
    }
}
