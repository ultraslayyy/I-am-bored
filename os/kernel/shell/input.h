#pragma once
#include <lib/stddef.h>

void handle_input_char(char c, char *input_buffer, size_t *input_pos);
void shell_handle_char(char c);
void update_shell_input(void);