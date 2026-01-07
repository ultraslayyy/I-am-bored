#include <lib/string.h>
#include <shell/alias.h>
#include <io/kernel_io.h>
#include <fs/fs.h>

void shell_init() {
    alias_init();
}

void process_command(char *cmd) {
    char expanded[128];
    const char *alias = alias_lookup(cmd);

    if (alias) {
        strcpy(expanded, alias);
        process_command(expanded);
        return;
    }
    if (strncmp(cmd, "alias", 5) == 0 && (cmd[5] == 0 ||cmd[5] == ' ')) {
        if (cmd[5] == 0) {
            alias_list();
            return;
        }

        char *eq = strchr(cmd + 6, '=');
        if (!eq) return;
        *eq = 0;
        alias_set(cmd + 6, eq + 1);
        return;
    }

    if (strcmp(cmd, "ls") == 0) {
        fs_ls("/");
        return;
    } else if (strncmp(cmd, "ls", 2) == 0) {
        fs_ls(cmd + 3);
        return;
    } else if (strncmp(cmd, "cat", 3) == 0) {
        fs_cat(cmd + 4);
        return;
    }
    
    if (strcmp(cmd, "help") == 0) {
        const char *text = "Commands: help, echo, cls, alias, ls, cat\n";
        for (size_t i = 0; text[i]; ++i)
            put_char(text[i], DEFAULT_ATTR);
    } else if (strncmp(cmd, "echo ", 4) == 0) {
        for (size_t i = 5; cmd[i]; ++i)
            put_char(cmd[i], DEFAULT_ATTR);
        put_char('\n', DEFAULT_ATTR);
    } else if (strcmp(cmd, "cls") == 0) {
        clear_screen();
    } else {
        const char *text = "Unknown command\n";
        for (size_t i = 0; text[i]; ++i)
            put_char(text[i], DEFAULT_ATTR);
    }
}