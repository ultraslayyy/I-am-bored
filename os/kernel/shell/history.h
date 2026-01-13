#pragma once

#define MAX_HISTORY 100
void add_history(const char *cmd);
int cmd_history(int argc, char **argv);