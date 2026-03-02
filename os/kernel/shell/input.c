#include <io/kernel_io.h>
#include <drivers/video/vga.h>
#include "shell.h"
#include <lib/string.h>
#include <fs/vfs.h>

void handle_input_char(char c, char *input_buffer, size_t *input_pos) {
    if (c == '\b') {
        if (*input_pos > 0) {
            (*input_pos)--;

            if (cursor_col > 0) {
                cursor_col--;
            } else if (cursor_row > 0) {
                cursor_row--;
                cursor_col = VGA_WIDTH - 1;
            }

            size_t row = ROW_IDX(cursor_row);

            text_buffer[row].chars[cursor_col] = ' ';
            text_buffer[row].attrs[cursor_col] = DEFAULT_ATTR;

            update_screen();
        }

        return;
    }

    if (c == '\n') {
        input_buffer[*input_pos] = 0;
        (*input_pos) = 0;

        cursor_col = 0;
        cursor_row++;
        total_lines = cursor_row + 1;

        scroll_viewport();
        update_screen();

        process_command(input_buffer);

        char path[MAX_PATH_LEN];
        vfs_get_path(kernel_cwd, path, sizeof(path));
        char prompt[MAX_PATH_LEN + MAX_INPUT];
        snprintf(prompt, sizeof(prompt), "%s $ ", path);

        put_string(prompt, DEFAULT_ATTR);
        return;
    }

    if (*input_pos < MAX_INPUT - 1) {
        input_buffer[(*input_pos)++] = c;
        put_char(c, 0x1F);
    }
}

static char s_input_buffer[MAX_INPUT];
static size_t s_input_pos = 0;

void shell_handle_char(char c) {
    handle_input_char(c, s_input_buffer, &s_input_pos);
}
