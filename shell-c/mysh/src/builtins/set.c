#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <errno.h>
#endif
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

#ifdef _WIN32
    if (!SetEnvironmentVariableA(name, value)) {
        printf("set: failed to set variable '%s'\n", name);
        return 1;
    }
#else
    if (setenv(name, value, 1) != 0) {
        printf("set: failed to set variable '%s': %s\n", name, strerror(errno));
        return 1;
    }
#endif

    return 0;
}