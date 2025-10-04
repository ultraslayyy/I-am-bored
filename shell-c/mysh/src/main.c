#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include <conio.h>
#include "utils.history.h"

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

        char *input_file = NULL;
        char *output_file = NULL;
        int append_mode = 0;

        for (int i = 0; i < argc; i++) {
            if (strcmp(args[i], "<") == 0 && i + 1 < argc) {
                input_file = args[i + 1];
                args[i] = NULL;
                for (int j = i + 2; j <= argc; j++) args[j - 2] = args[j];
                argc -= 2;
                i--;
            } else if (strcmp(args[i], ">") == 0 && i + 1 < argc) {
                output_file = args[i + 1];
                args[i] = NULL;
                for (int j = i + 2; j <= argc; j++) args[j - 2] = args[j];
                argc -= 2;
                i--;
            } else if (strcmp(args[i], ">>") == 0 && i + 1 < argc) {
                output_file = args[i + 1];
                append_mode = 1;
                for (int j = i + 2; j <= argc; j++) args[j - 2] = args[j];
                argc -= 2;
                i--;
            }
        }

        HANDLE hInput = NULL, hOutput = NULL;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        if (input_file) {
            hInput = CreateFile(
                input_file,
                GENERIC_READ,
                FILE_SHARE_READ,
                &sa,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hInput == INVALID_HANDLE_VALUE) {
                printf("Cannot open input file: %s\n", input_file);
                continue;
            }
        }

        if (output_file) {
            if (append_mode) {
                hOutput = CreateFile(
                    output_file,
                    FILE_APPEND_DATA,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    &sa,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );
            } else {
                hOutput = CreateFile(
                    output_file,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    &sa,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );
            }

            if (hOutput == INVALID_HANDLE_VALUE) {
                printf("Cannot open output file: %s\n", output_file);
                if (hInput) CloseHandle(hInput);
                continue;
            }
        }

        if (strcmp(args[0], "exit") == 0) {
            builtin_exit();
            continue;
        } else if (strcmp(args[0], "cd") == 0) {
            builtin_cd(argc, args);
            continue;
        } else if (strcmp(args[0], "pwd") == 0) {
            builtin_pwd();
            continue;
        } else if (strcmp(args[0], "history") == 0) {
            builtin_history();
            continue;
        }

        // Startup process
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdInput = input_file ? hInput : GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = output_file ? hOutput : GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        ZeroMemory(&pi, sizeof(pi));

        char cmdline[MAX_INPUT] = "";
        for (int i = 0; i < argc; i++) {
            strcat(cmdline, args[i]);
            if (i < argc - 1)
                strcat(cmdline, " ");
        }

        if (!CreateProcess(
            NULL,
            cmdline,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            printf("Failed to run command: %d\n", GetLastError());
            if (hInput) CloseHandle(hInput);
            if (hOutput) CloseHandle(hOutput);
            continue;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (hInput) CloseHandle(hInput);
        if (hOutput) CloseHandle(hOutput);
    }

    return 0;
}