#pragma once
#include <lib/stddef.h>

#define MAX_INPUT 128
#define HIST_USER_TYPED 1
#define HIST_ALIAS 2

extern int kbd_shell_control;

void process_command(char *cmd, ...);