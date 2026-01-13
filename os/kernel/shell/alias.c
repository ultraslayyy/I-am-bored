#include <lib/string.h>
#include <io/kernel_io.h>
#include "alias.h"

typedef struct {
    char name[MAX_ALIAS_NAME];
    char value[MAX_ALIAS_VALUE];
} Alias;

static Alias aliases[MAX_ALIASES];
static size_t alias_count = 0;

void alias_init(void) {
    alias_count = 0;
}

void alias_set(const char *name, const char *value) {
    for (size_t i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            strcpy(aliases[i].value, value);
            return;
        }
    }

    if (alias_count < MAX_ALIASES) {
        strcpy(aliases[alias_count].name, name);
        strcpy(aliases[alias_count].value, value);
        alias_count++;
    }
}

void alias_unset(const char *name) {
    for (size_t i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            for (size_t j = i; j < alias_count - 1; ++j) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            return;
        }
    }
}

const char *alias_lookup(const char *name) {
    for (size_t i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return 0;
}

void alias_list() {
    for (size_t i = 0; i < alias_count; ++i) {
        char buf[MAX_ALIAS_NAME + MAX_ALIAS_VALUE];
        snprintf(buf, sizeof(buf), "alias %s=%s\n", aliases[i].name, aliases[i].value);
        put_string(buf, DEFAULT_ATTR);
    }
}

int cmd_alias(int argc, char **argv) {
    if (argc == 1) {
        alias_list();
        return 0;
    } else if (argc == 2) {
        char *eq = strchr(argv[1], '=');
        if (!eq) {
            put_string("A value is required", DEFAULT_ATTR);
            return 1;
        }
        *eq = 0;
        alias_set(argv[1], eq + 1);
        return 0;
    } else {
        put_string("Too many parameters\n", DEFAULT_ATTR);
        return 1;
    }
}