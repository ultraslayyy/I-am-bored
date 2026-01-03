#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#endif

#include "builtins.h"
#include "utils.history.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

int parse_args(char *input, char **args, int max_args) {
    int argc = 0;
    char *p = input;

    while (*p && argc < max_args - 1) {
        while (*p == ' ') p++;
        if (!*p) break;

        char *start = p;
        int in_quotes = 0;
        char quote_char = 0;

        while (*p) {
            if (in_quotes) {
                if (*p == quote_char) {
                    in_quotes = 0;
                }
            } else {
                if (*p == '"' || *p == '\'') {
                    in_quotes = 1;
                    quote_char = *p;
                } else if (*p == ' ') {
                    break;
                }
            }
            p++;
        }

        int len = p - start;
        char *arg = (char *)malloc(len + 1);
        strncpy(arg, start, len);
        arg[len] = '\0';

        if ((arg[0] == '"' && arg[len - 1] == '"') ||
            (arg[0] == '\'' && arg[len - 1] == '\'')) {
            arg[len - 1] = '\0';
            memmove(arg, arg + 1, len - 1);
        }

        args[argc++] = arg;

        if (*p == ' ') p++;
    }

    args[argc] = NULL;
    return argc;
}

int main() {
    char input[MAX_INPUT];

    while (1) {
        char cwd[MAX_INPUT];

#ifdef _WIN32
        GetCurrentDirectory(MAX_INPUT, cwd);
#else
        if (!getcwd(cwd, sizeof(cwd))) {
            strncpy(cwd, "?", sizeof(cwd));
        }
#endif

        char prompt[MAX_INPUT];
        snprintf(prompt, sizeof(prompt), "\033[32m%s\033[0m$ ", cwd);
        read_input_with_history(input, sizeof(input), prompt);

        input[strcspn(input, "\r\n")] = 0;

        if (strlen(input) == 0) {
            continue;
        }

        char *args[MAX_ARGS];
        int argc = parse_args(input, args, MAX_ARGS);
        if (argc == 0) continue;

        const char *alias_val = alias_lookup(args[0]);
        if (alias_val) {
            char expanded[MAX_INPUT];
            snprintf(expanded, sizeof(expanded), "%s", alias_val);

            char combined[MAX_INPUT];
            snprintf(combined, sizeof(combined), "%s", expanded);
            for (int i = 1; i < argc; i++) {
                strcat(combined, " ");
                strcat(combined, args[i]);
            }

            argc = parse_args(combined, args, MAX_ARGS);
        }

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

#ifdef _WIN32
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
        } else if (strcmp(args[0], "alias") == 0) {
            builtin_alias(argc, args);
            continue;
        } else if (strcmp(args[0], "unalias") == 0) {
            builtin_unalias(argc, args);
            continue;
        } else if (strcmp(args[0], "set") == 0) {
            builtin_set(argc, args);
            continue;
        } else if (strcmp(args[0], "unset") == 0) {
            builtin_unset(argc, args);
        } else if (strcmp(args[0], "env") == 0) {
            builtin_env();
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

        BOOL success = CreateProcess(
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
        );

        if (!success && GetLastError() == ERROR_FILE_NOT_FOUND) {
            char wrapped[MAX_INPUT * 2];
            snprintf(wrapped, sizeof(wrapped), "cmd.exe /C %s", cmdline);

            success = CreateProcess(
                NULL,
                wrapped,
                NULL,
                NULL,
                TRUE,
                0,
                NULL,
                NULL,
                &si,
                &pi
            );

            if (!success) {
                printf("Failed to run command: %d\n", GetLastError());
                if (hInput) CloseHandle(hInput);
                if (hOutput) CloseHandle(hOutput);
                continue;
            }
        } else if (!success) {
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
#else
        int in_fd = -1, out_fd = -1

        if (input_file) {
            in_fd = open(input_file, O_RDONLY);
            if (in_fd < 0) {
                fprintf(stderr, "Cannot open input file: %s: %s\n", input_file, sterror(errno));
                for (int i = 0; i < argc; i++) {
                    free(args[i]);
                }
                continue;
            }
        }

        if (output_file) {
            if (append_mode) {
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
            } else {
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            if (out_fd < 0) {
                fprintf(stderr, "Cannot open output file: %s: %s\n", output_file, strerror(errno));
                if (in_vd >= 0) close(in_fd);
                for (int i = 0; i < argc; i++) {
                    free(args[i]);
                }
                continue;
            }
        }

        char cmdline[MAX_INPUT] = "";
        for (int i = 0; i < argc; i++) {
            strncat(cmdline, args[i], sizeof(cmdline) - strlen(cmdline) - 1);
            if (i < argc - 1) strncat(cmdline, "", sizeof(cmdline) - strlen(cmdline) - 1);
        }

        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            if (in_fd >= 0) close(in_fd);
            if (out_fd >= 0) close(out_fd);
            for (int i = 0; i < argc; i++) {
                free(args[i]);
            }
            continue;
        } else if (pid == 0) {
            if (in_fd >= 0) {
                if (dup2(in_fd, STDIN_FILENO) < 0) {
                    fprintf(stderr, "dup2 stdin failed: %s\n", strerror(errno));
                    _exit(127);
                }
            }
            if (out_fd >= 0) {
                if (dup2(out_fd, STDOUT_FILENO) < 0) {
                    fprintf(stderr, "dup2 stdout failed: %s\n", strerror(errno));
                    _exit(127);
                }
            }

            if (in_fd >= 0) close(in_fd);
            if (out_fd >= 0) close(out_fd);

            execvp(args[0], args);
            execlp("sh", "sh", "-c", cmdline, (char *)NULL);
            fprintf(stderr, "Failed to run command: %s\n", strerror(errno));
            _exit(127);
        } else {
            int status = 0;
            waitpid(pid, &status, 0);
            if (in_fd >= 0) close(in_fd);
            if (out_fd >= 0) close(out_fd);
        }
#endif

        for (int i = 0; i < argc; i++) {
            free(args[i]);
        }
    }

    return 0;
}