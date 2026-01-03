#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <errno.h>
#endif
#include "builtins.h"

int builtin_unset(int argc, char **argv) {
    if (argc == 1) {
        return builtin_env();
    }

    char *name = argv[1];

#ifdef _WIN32
    if (!SetEnvironmentVariableA(name, NULL)) {
        printf("unset: failed to unset variable '%s'\n", name);
        return 1;
    }
#else
    if (unsetenv(name) != 0) {
        printf("unset: failed to unset variable '%s': %s\n", name, strerror(errno));
        return 1;
    }
#endif

    return 0;
}