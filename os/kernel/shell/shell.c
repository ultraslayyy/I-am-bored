#include <arch.h>
#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <lib/stdarg.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <shell/alias.h>

#include "cat.h"
#include "cd.h"
#include "echo.h"
#include "history.h"
#include "ls.h"
#include "mkdir.h"
#include "mouse.h"
#include "pci.h"
#include "ping.h"
#include "rm.h"
#include "shell.h"
#include "shutdown.h"
#include "touch.h"

int kbd_shell_control = 1;

int cmd_help(int argc, char **argv);
int cmd_cls(int argc, char **argv);

size_t recur_level = 0;

typedef int (*command_fn)(int argc, char **argv);

typedef struct {
    const char *name;
    command_fn  handler;
    const char *help;
} command_t;

static command_t commands[] = {
    {"help",     cmd_help,     "Show this help"},
    {"alias",    cmd_alias,    "Alias idk"},
    {"cat",      cmd_cat,      "Print file contents"},
    {"cd",       cmd_cd,       "Change the Current Working Directory"},
    {"cls",      cmd_cls,      "Clear screen"},
    {"echo",     cmd_echo,     "Print text"},
    {"history",  cmd_history,  "Print command history"},
    {"ls",       cmd_ls,       "List directory contents"},
    {"mkdir",    cmd_mkdir,    "Create a new directory"},
    {"mouse",    cmd_mouse,    "Enable and disable mouse (for testing only)"},
    {"pci",      cmd_pci,      "List PCI devices"},
    {"ping",     cmd_ping,     "Ping an IP/domain"},
    {"rm",       cmd_rm,       "Remove a file/directory"},
    {"shutdown", cmd_shutdown, "Shutdown computer (currently QEMU only)"},
    {"touch",    cmd_touch,    "Create a new file"}
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

int cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;

    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        char text[128];
        snprintf(text, sizeof(text), "%s - %s\n", commands[i].name, commands[i].help);
        put_string(text, DEFAULT_ATTR);
    }
    return 0;
}

int cmd_cls(int argc, char **argv) {
    (void)argc;
    (void)argv;

    clear_screen();
    return 0;
}

int parse_args(char *cmd, char **argv, int max) {
    int argc = 0;
    char *p = cmd;
    char *tok;

    while ((tok = strktok(&p, " \t")) && argc < max) {
        argv[argc++] = tok;
    }

    return argc;
}

void process_command(char *cmd, ...) {
    kbd_shell_control = 0;
    if (cmd[0] != 0) {
        va_list args;
        va_start(args, cmd);
        int type = va_arg(args, int);

        if (type != HIST_ALIAS) {
            add_history(cmd);
        }
    }

    char expanded[128];
    char *argv[16];
    int argc;

    if (cmd[0] == '.' && (cmd[1] == '/' || cmd[1] == '\\')) {
        fs_node_t *elf = vfs_resolve(cmd);
        if (elf == NULL) {
            put_string("File not found\n", DEFAULT_ATTR);
            kbd_shell_control = 1;
            return;
        }

        char *file_data = malloc(elf->size);
        size_t read = vfs_read(elf, 0, file_data, elf->size);
        if (read != elf->size) {
            put_string("short read\n", DEFAULT_ATTR);
        }

        load_elf((uint8_t *)file_data);
        kbd_shell_control = 1;
        return;
    }

    const char *alias = alias_lookup(cmd);
    if (alias) {
        if (recur_level++ > MAX_ALIAS_RECURSION) {
            put_string("Alias recursion limit exceeded\n", DEFAULT_ATTR);
            recur_level--;
            kbd_shell_control = 1;
            return;
        }
        strcpy(expanded, alias);
        process_command(expanded, HIST_ALIAS);
        recur_level--;
        kbd_shell_control = 1;
        return;
    }

    argc = parse_args(cmd, argv, 16);
    if (argc == 0) {
        kbd_shell_control = 1;
        return;
    }

    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv);
            kbd_shell_control = 1;
            return;
        }
    }

    put_string("Unknown command\n", DEFAULT_ATTR);
    kbd_shell_control = 1;
}