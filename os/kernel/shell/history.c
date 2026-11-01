#include <lib/stddef.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include "history.h"
#include "shell.h"

static char history[MAX_HISTORY][MAX_INPUT];
static size_t history_count = 0;
static size_t history_start = 0;
static size_t history_index = 0;

void add_history(const char *cmd) {
    size_t index = (history_start + history_count) % MAX_HISTORY;

    strlcpy(history[index], cmd, MAX_INPUT);

    if (history_count < MAX_HISTORY) {
        history_count++;
    } else {
        history_start = (history_start + 1) % MAX_HISTORY;
    }

    history_index = history_count;
}

void list_history(void) {
    char buf[128];
    for (size_t i = 0; i < history_count; ++i) {
        size_t index = (history_start + i) % MAX_HISTORY;
        snprintf(buf, sizeof(buf), "  %u  %s\n", i + 1, history[index]);
        put_string(buf, DEFAULT_ATTR);
    }
}

void clear_history(void) {
    history_count = 0;
    history_start = 0;
    history_index = 0;
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

const char *history_previous(void) {
    if (history_count == 0) {
        return NULL;
    }

    if (history_index == 0) {
        return history[history_start];
    }

    history_index--;

    size_t index = (history_start + history_index) % MAX_HISTORY;
    return history[index];
}

const char *history_next(void) {
    if (history_index >= history_count) {
        return NULL;
    }

    history_index++;

    if (history_index == history_count) {
        return shell_saved_input;
    }

    size_t index = (history_start + history_index) % MAX_HISTORY;
    return history[index];
}

void history_reset_navigation(void) {
    history_index = history_count;
}

int history_is_navigating(void) {
    return history_index < history_count;
}