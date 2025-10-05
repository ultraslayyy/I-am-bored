#include <windows.h>
#include <stdio.h>

int builtin_env() {
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
    return 0;
}