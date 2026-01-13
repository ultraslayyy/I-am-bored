#include <lib/stddef.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include "history.h"
#include "shell.h"

static char history[MAX_HISTORY][MAX_INPUT];
static size_t history_count = 0;
static size_t history_start = 0;

void add_history(const char *cmd) {
    size_t index = (history_start + history_count) % MAX_HISTORY;

    strlcpy(history[index], cmd, MAX_INPUT);

    if (history_count < MAX_HISTORY) {
        history_count++;
    } else {
        history_start = (history_start + 1) % MAX_HISTORY;
    }
}

void list_history() {
    char buf[128];
    for (size_t i = 0; i < history_count; ++i) {
        size_t index = (history_start + i) % MAX_HISTORY;
        snprintf(buf, sizeof(buf), "  %u  %s\n", i + 1, history[index]);
        put_string(buf, DEFAULT_ATTR);
    }
}

void clear_history() {
    history_count = 0;
    history_start = 0;
}

int cmd_history(int argc, char **argv) {
    if (argc == 1 || strcmp(argv[1], "list") == 0) {
        list_history();
        return 0;
    }
    
    if (strcmp(argv[1], "clear") == 0) {
        clear_history();
        return 0;
    }

    put_string("Usage: history [list|clear]\n", DEFAULT_ATTR);
    return 1;
}