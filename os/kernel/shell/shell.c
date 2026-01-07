#include <lib/string.h>
#include <shell/alias.h>
#include <io/kernel_io.h>

void shell_init() {
    alias_init();
}

void process_command(char *cmd, size_t *cursor_pos) {
    char expanded[128];
    const char *alias = alias_lookup(cmd);

    if (alias) {
        strcpy(expanded, alias);
        process_command(expanded, cursor_pos);
        return;
    }
    if (strncmp(cmd, "alias", 5) == 0 && (cmd[5] == 0 ||cmd[5] == ' ')) {
        if (cmd[5] == 0) {
            alias_list(cursor_pos);
            return;
        }

        char *eq = strchr(cmd + 6, '=');
        if (!eq) return;
        *eq = 0;
        alias_set(cmd + 6, eq + 1);
        return;
    }

    if (strcmp(cmd, "help") == 0) {
        const char *text = "Commands: help, echo, cls, alias\n";
        for (size_t i = 0; text[i]; ++i)
            put_char(text[i], cursor_pos, 0x07);
    } else if (strncmp(cmd, "echo ", 4) == 0) {
        for (size_t i = 5; cmd[i]; ++i)
            put_char(cmd[i], cursor_pos, 0x07);
        put_char('\n', cursor_pos, 0x07);
    } else if (strcmp(cmd, "cls") == 0) {
        clear_screen(cursor_pos);
    } else {
        const char *text = "Unknown command\n";
        for (size_t i = 0; text[i]; ++i)
            put_char(text[i], cursor_pos, 0x07);
    }
}