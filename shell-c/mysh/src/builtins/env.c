#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

int builtin_env() {
#ifdef _WIN32
    LPWCH envStrings = GetEnvironmentStringsW();
    if (!envStrings) {
        printf("env: failed to get environment string\n");
        return 1;
    }

    LPWSTR var = envStrings;
    while (*var) {
        wprintf(L"%ls\n", var);
        var += wcslen(var) + 1;
    }

    FreeEnvironmentStringsW(envStrings);
#else
    extern char **environ;
    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
#endif
    return 0;
}