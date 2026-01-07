#include <io/kernel_io.h>
#include <shell/shell.h>
#include <lib/string.h>

void handle_input_char(char c, char *input_buffer, size_t *input_pos, const char *prompt, size_t *cursor_pos) {
    if (c == '\b' && *input_pos > 0) {
        (*input_pos)--;
        if (*cursor_pos > 0) {
            (*cursor_pos)--;
            put_char(' ', cursor_pos, 0x07);
            (*cursor_pos)--;
        }
        return;
    }

    if (c == '\n') {
        put_char('\n', cursor_pos, 0x07);
        input_buffer[*input_pos] = 0;
        process_command(input_buffer, cursor_pos);
        *input_pos = 0;
        put_string(prompt, cursor_pos, 0x07);
        return;
    }

    if (*input_pos < MAX_INPUT - 1) {
        input_buffer[(*input_pos)++] = c;
        put_char(c, cursor_pos, 0x1F);
    }
}
