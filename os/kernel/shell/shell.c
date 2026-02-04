#include <lib/string.h>
#include <shell/alias.h>
#include <io/kernel_io.h>
#include <lib/stdarg.h>
#include <fs/fs.h>
#include "shutdown.h"
#include "shell.h"
#include "history.h"
#include "pci.h"
#include "cd.h"

int cmd_help(int argc, char **argv);
int cmd_echo(int argc, char **argv);
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
    {"cls",      cmd_cls,      "Clear screen"},
    {"echo",     cmd_echo,     "Print text"},
    {"history",  cmd_history,  "Print command history"},
    {"ls",       cmd_ls,       "List directory contents"},
    {"pci",      cmd_pci,      "List PCI devices"},
    {"shutdown", cmd_shutdown, "Shutdown computer (currently QEMU only)"},
    {"cd",       cmd_cd,       "Change the Current Working Directory"}
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

int cmd_echo(int argc, char **argv) {
    char buf[MAX_INPUT];
    size_t len = 0;
    buf[0] = '\0';

    for (size_t i = 1; (int)i < argc; ++i) {
        len += snprintf(buf + len, sizeof(buf) - len, "%s%s", argv[i], ((int)i + 1 < argc ? " " : ""));
        if (len >= sizeof(buf)) {
            break;
        }
    }

    put_string(buf, DEFAULT_ATTR);
    put_char('\n', DEFAULT_ATTR);
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

    const char *alias = alias_lookup(cmd);
    if (alias) {
        if (recur_level++ > MAX_ALIAS_RECURSION) {
            put_string("Alias recursion limit exceeded\n", DEFAULT_ATTR);
            recur_level--;
            return;
        }
        strcpy(expanded, alias);
        process_command(expanded, HIST_ALIAS);
        recur_level--;
        return;
    }

    argc = parse_args(cmd, argv, 16);
    if (argc == 0) return;

    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv);
            return;
        }
    }

    put_string("Unknown command\n", DEFAULT_ATTR);
}