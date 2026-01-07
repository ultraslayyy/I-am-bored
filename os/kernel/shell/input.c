#include <io/kernel_io.h>
#include <drivers/video/vga.h>
#include <shell/shell.h>
#include <lib/string.h>

void handle_input_char(char c, char *input_buffer, size_t *input_pos, const char *prompt) {
    if (c == '\b') {
        if (*input_pos > 0) {
            (*input_pos)--;

            if (cursor_col > 0) {
                cursor_col--;
            } else if (cursor_row > 0) {
                cursor_row--;
                cursor_col = VGA_WIDTH - 1;
            }

            text_buffer[cursor_row].chars[cursor_col] = ' ';
            text_buffer[cursor_row].attrs[cursor_col] = DEFAULT_ATTR;

            update_screen();
        }
        return;
    }

    if (c == '\n') {
        input_buffer[*input_pos] = 0;
        (*input_pos) = 0;

        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= BUFFER_HEIGHT) cursor_row = BUFFER_HEIGHT - 1;
        if (cursor_row >= buffer_lines) buffer_lines = cursor_row + 1;

        scroll_viewport();
        update_screen();

        process_command(input_buffer);
        put_string(prompt, DEFAULT_ATTR);
        return;
    }

    if (*input_pos < MAX_INPUT - 1) {
        input_buffer[(*input_pos)++] = c;
        put_char(c, 0x1F);
    }
}
