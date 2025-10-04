#include <windows.h>
#include <conio.h>
#include <stdio.h>

#define MAX_HISTORY 100
#define MAX_INPUT 1024

char history[MAX_HISTORY][MAX_INPUT];
int history_count = 0;
int history_index = -1;

void add_history(const char* cmd) {
    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count++], cmd);
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[MAX_HISTORY - 1], cmd);
    }
}

void read_input_with_history(char *buffer, size_t size, const char *prompt) {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    printf("%s", prompt);
    fflush(stdout);

    int pos = 0;
    buffer[0] = '\0';
    history_index = history_count;

    INPUT_RECORD record;
    DWORD count;

    while (1) {
        ReadConsoleInput(hStdin, &record, 1, &count);

        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            int vk = record.Event.KeyEvent.wVirtualKeyCode;

            if (vk == VK_RETURN) {
                putchar('\n');
                buffer[pos] = '\0';
                if (strlen(buffer) > 0) add_history(buffer);
                return;
            }
            else if (vk == VK_BACK) {
                if (pos > 0) {
                    pos--;
                    buffer[pos] = '\0';
                    printf("\b \b");
                    fflush(stdout);
                }
            }
            else if (vk == VK_UP) {
                if (history_index > 0) {
                    history_index--;
                    printf("\r\033[K");
                    printf("%s%s", prompt, history[history_index]);
                    fflush(stdout);
                    strcpy(buffer, history[history_index]);
                    pos = strlen(buffer);
                }
            }
            else if (vk == VK_DOWN) {
                if (history_index < history_count - 1) {
                    history_index++;
                    printf("\r\033[K");
                    printf("%s%s", prompt, history[history_index]);
                    fflush(stdout);
                    strcpy(buffer, history[history_index]);
                    pos = strlen(buffer);
                } else {
                    history_index = history_count;
                    printf("\r\033[K%s", prompt);
                    fflush(stdout);
                    buffer[0] = '\0';
                    pos = 0;
                }
            }
            else {
                char c = record.Event.KeyEvent.uChar.AsciiChar;
                if (c >= 32 && pos < size - 1) {
                    buffer[pos++] = c;
                    buffer[pos] = '\0';
                    putchar(c);
                    fflush(stdout);
                }
            }
        }
    }
}

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%4d  %s\n", i + 1, history[i]);
    }
}