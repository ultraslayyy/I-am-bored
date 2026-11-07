#include <fs/vfs.h>
#include <io/kernel_io.h>
#include <lib/stdarg.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <shell/alias.h>
#include <arch.h>

#include "command_task.h"

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
#include "time.h"
#include "touch.h"
#include "ussr.h"

int kbd_shell_control = 1;

int cmd_help(int argc, char **argv);
int cmd_cls(int argc, char **argv);

size_t recur_level = 0;

typedef struct {
    const char       *name;
    command_handler_t handler;
    const char       *help;
} command_t;

// test
#include <timer/timer.h>
int cmd_test(int argc, char **argv) {
    (void)argc;
    (void)argv;
    while (1) {
        put_string("A", DEFAULT_ATTR);
        delay_ms(50);
    }
}

static command_t commands[] = {
    {"help",     cmd_help,     "Show this help"},
    {"alias",    cmd_alias,    "Alias idk"},
    {"cat",      cmd_cat,      "Print file contents"},
    {"cd",       cmd_cd,       "Change the Current Working Directory"},
    {"cls",      cmd_cls,      "CLear Screen"},
    {"echo",     cmd_echo,     "Print text"},
    {"history",  cmd_history,  "Print command history"},
    {"ls",       cmd_ls,       "List directory contents"},
    {"mkdir",    cmd_mkdir,    "Create a new directory"},
    {"mouse",    cmd_mouse,    "Enable and disable mouse (for testing only)"},
    {"pci",      cmd_pci,      "List PCI devices"},
    {"ping",     cmd_ping,     "Ping an IP/domain"},
    {"reboot",   cmd_reboot,   "Reboot"},
    {"rm",       cmd_rm,       "Remove a file/directory"},
    {"shutdown", cmd_shutdown, "Shutdown computer (currently QEMU only)"},
    {"time",     cmd_time,     "Show current time"},
    {"touch",    cmd_touch,    "Create a new file"},
    {"ussr",     cmd_ussr,     "Need I say more?"},

    {"test",     cmd_test,     "Test rq"}
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
        va_end(args);
        if (type != HIST_ALIAS) {
            add_history(cmd);
        }
    }

    char expanded[128];

    if (cmd[0] == '.' && (cmd[1] == '/' || cmd[1] == '\\')) {
        fs_node_t *elf = vfs_resolve(cmd);
        if (elf == NULL) {
            put_string("File not found\n", DEFAULT_ATTR);
            goto exit;
        }

        char *file_data = malloc(elf->size);
        size_t read = vfs_read(elf, 0, file_data, elf->size);
        if (read != elf->size) {
            put_string("short read\n", DEFAULT_ATTR);
        }

        //load_elf((uint8_t *)file_data);
        put_string("ELF execution disabled\n", DEFAULT_ATTR);
        free(file_data);
        goto exit;
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
        goto exit;
    }

    command_context_t *ctx = malloc(sizeof(command_context_t));
    if (!ctx) {
        put_string("Failed to create context\n", DEFAULT_ATTR);
        goto exit;
    }

    if (strlen(cmd) >= sizeof(ctx->cmd)) {
        put_string("Command too long\n", DEFAULT_ATTR);
        free(ctx);
        goto exit;
    }
    strcpy(ctx->cmd, cmd);

    ctx->argc = parse_args(ctx->cmd, ctx->argv, 16);

    if (ctx->argc == 0) {
        free(ctx);
        goto exit;
    }

    ctx->handler = NULL;

    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        if (strcmp(ctx->argv[0], commands[i].name) == 0) {
            ctx->handler = commands[i].handler;
            break;
        }
    }

    if (!ctx->handler) {
        put_string("Unknown command\n", DEFAULT_ATTR);
        free(ctx);
        goto exit;
    }

    uint32_t flags = irq_save();

    uint32_t pid = task_create(command_task, ctx, free);
    if (!pid) {
        put_string("Failed to create task\n", DEFAULT_ATTR);
        free(ctx);
        goto exit;
    }

    foreground_task = pid;
    irq_restore(flags);

    while (task_exists(pid)) {
        __asm__ volatile("sti; hlt");
    }

    foreground_task = 0;

exit:
    kbd_shell_control = 1;
    return;
}