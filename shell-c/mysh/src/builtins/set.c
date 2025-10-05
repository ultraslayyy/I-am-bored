#include <windows.h>
#include <stdio.h>
#include "builtins.h"

int builtin_set(int argc, char **argv) {
    if (argc == 1) {
        return builtin_env();
    }

    char *eq = strchr(argv[1], '=');
    if (!eq) {
        printf("set: invalid format, use NAME=VALUE\n");
        return 1;
    }

    *eq = '\0';
    char *name = argv[1];
    char *value = eq + 1;
    if (!SetEnvironmentVariableA(name, value)) {
        printf("set: failed to set variable '%s'\n", name);
        return 1;
    }

    return 0;
}