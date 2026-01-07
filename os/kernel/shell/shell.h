#pragma once
#include <stddef.h>

#define MAX_INPUT 128

void shell_init(void);
void process_command(char *cmd, size_t *cursor_pos);