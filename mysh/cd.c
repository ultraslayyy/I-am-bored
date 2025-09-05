#include <windows.h>
#include <stdio.h>
#include "builtins.h"

int builtin_cd(int argc, char **argv) {
    if (argc < 2) {
        printf("cd: missing argument\n");
        return 1;
    }
    if (!SetCurrentDirectory(argv[1])) {
        printf("cd: cannot change directory to '%s'\n", argv[1]);
        return 1;
    }
    return 0;
}