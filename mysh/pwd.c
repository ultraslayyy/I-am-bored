#include <windows.h>
#include <stdio.h>
#include "builtins.h"

int builtin_pwd() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, buffer)) {
        printf("Current Directory: %s\n", buffer);
    } else {
        printf("Error retrieving current directory.\n");
    }
    return 0;
}