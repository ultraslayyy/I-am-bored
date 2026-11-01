#pragma once

#define MAX_HISTORY 100
void add_history(const char *cmd);
int cmd_history(int argc, char **argv);

const char *history_previous(void);
const char *history_next(void);
void history_reset_navigation(void);
int history_is_navigating(void);