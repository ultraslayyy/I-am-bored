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
    for (size_t i = 0; i < alias_count; i++) {
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
    for (size_t i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            for (size_t j = i; j < alias_count - 1; j++)
                aliases[j] = aliases[j + 1];
            alias_count--;
            return;
        }
    }
}

const char *alias_lookup(const char *name) {
    for (size_t i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0)
            return aliases[i].value;
    }
    return 0;
}

void alias_list() {
    for (size_t i = 0; i < alias_count; i++) {
        const char *p = "alias ";
        while (*p) put_char(*p++, DEFAULT_ATTR);

        p = aliases[i].name;
        while (*p) put_char(*p++, DEFAULT_ATTR);

        put_char('=', DEFAULT_ATTR);

        p = aliases[i].value;
        while (*p) put_char(*p++, DEFAULT_ATTR);

        put_char('\n', DEFAULT_ATTR);
    }
}
