#pragma once

typedef int (*command_handler_t)(int argc, char **argv);

typedef struct {
    command_handler_t handler;

    int argc;

    char cmd[128];
    char *argv[16];
} command_context_t;

void command_task(void *arg);