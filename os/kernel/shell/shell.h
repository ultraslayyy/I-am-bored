#pragma once
#include <lib/stddef.h>

#define MAX_INPUT 128
#define HIST_USER_TYPED 1
#define HIST_ALIAS 2

void shell_init(void);
void process_command(char *cmd, ...);