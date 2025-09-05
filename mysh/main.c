#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include <conio.h>
#include "utils/utils.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];

    while (1) {
        char cwd[MAX_INPUT];
        GetCurrentDirectory(MAX_INPUT, cwd);
        
        char prompt[MAX_INPUT];
        snprintf(prompt, sizeof(prompt), "\033[32m%s\033[0m$ ", cwd);
        read_input_with_history(input, sizeof(input), prompt);

        input[strcspn(input, "\r\n")] = 0;

        if (strlen(input) == 0) {
            continue;
        }

        char *args[MAX_ARGS];
        int argc = 0;

        char *token = strtok(input, " ");
        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (strcmp(args[0], "exit") == 0) {
            builtin_exit(argc, args);
            continue;
        } else if (strcmp(args[0], "cd") == 0) {
            builtin_cd(argc, args);
            continue;
        }

        char cmdline[MAX_INPUT] = "";
        for (int i = 0; i < argc; i++) {
            strcat(cmdline, args[i]);
            if (i < argc - 1)
                strcat(cmdline, " ");
        }

        // Startup process
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcess(
            NULL,
            cmdline,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            printf("Failed to run command: %d\n", GetLastError());
            continue;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}