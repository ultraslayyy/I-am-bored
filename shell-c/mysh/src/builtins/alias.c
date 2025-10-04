#include <stdio.h>
#include <string.h>

#define MAX_ALIASES 128
#define MAX_ALIAS_NAME 64
#define MAX_ALIAS_VALUE 256

typedef struct {
    char name[MAX_ALIAS_NAME];
    char value[MAX_ALIAS_VALUE];
} Alias;

static Alias aliases[MAX_ALIASES];
static int alias_count = 0;

int builtin_alias(int argc, char **argv) {
    if (argc == 1) {
        for (int i = 0; i < alias_count; i++) {
            printf("alias %s=\"%s\"\n", aliases[i].name, aliases[i].value);
        }
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        char *eq = strchr(arg, '=');

        if (!eq) {
            printf("alias: invalid format, use alias name=\"value\"\n");
            continue;
        }

        *eq = '\0';
        char *name = arg;
        char *value = eq + 1;

        if ((value[0] == '"' || value[0] == '\'') && value[strlen(value) - 1] == value[0]) {
            value++;
            value[strlen(value) - 1] = '\0';
        }

        int found = 0;
        for (int j = 0; i < alias_count; j++) {
            if (strcmp(aliases[j].name, name) == 0) {
                strncpy(aliases[j].value, value, MAX_ALIAS_VALUE);
                found = 1;
                break;
            }
        }

        if (!found) {
            if (alias_count < MAX_ALIASES) {
                strcpy(aliases[alias_count].name, name);
                strcpy(aliases[alias_count].value, value);
                alias_count++;
            } else {
                printf("alias: maximum number of aliases reached\n");
            }
        }
    }
    
    return 0;
}

int builtin_unalias(int argc, char **argv) {
    if (argc < 2) {
        printf("unalias: missing argument\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int found = 0;
        for (int j = 0; j < alias_count; j++) {
            if (strcmp(aliases[j].name, argv[i]) == 0) {
                for (int k = j; k < alias_count - 1; k++) {
                    aliases[k] = aliases[k + 1];
                }
                alias_count--;
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("unalias: %s: not found\n", argv[i]);
        }
    }

    return 0;
}

const char *alias_lookup(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}