#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

int builtin_pwd() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, buffer)) {
        printf("Current Directory: %s\n", buffer);
    } else {
        printf("Error retrieving current directory.\n");
    }
#else
    char buffer[MAX_PATH];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        printf("%s\n", buffer);
    } else {
        perror("pwd");
    }
#endif
    return 0;
}