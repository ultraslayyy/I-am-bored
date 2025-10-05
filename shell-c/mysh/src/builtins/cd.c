#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

int builtin_cd(int argc, char **argv) {
    if (argc < 2) {
        printf("cd: missing argument\n");
        return 1;
    }
#ifdef _WIN32
    if (!SetCurrentDirectory(argv[1])) {
        printf("cd: cannot change directory to '%s'\n", argv[1]);
        return 1;
    }
#else
    if (chdir(argv[1]) != 0) {
        printf("cd: cannot change directory to '%s': '%s'\n", argv[1], strerror(errno));
        return 1;
    }
#endif
    return 0;
}