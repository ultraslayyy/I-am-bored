#include <stddef.h>
#ifndef UTILS_HISTORY_H
#define UTILS_HISTORY_H

void read_input_with_history(char *buffer, size_t size, const char *prompt);
void print_history();

#endif