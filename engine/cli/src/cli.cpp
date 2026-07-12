#include "cli.h"

#include <iostream>
#include <conio.h>

CLI::CLI() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, mode);
}

void CLI::goToXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

COORD CLI::getCursorPosition() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition;
    }
    return {0,0};
}

std::string CLI::MultipleChoice(const std::vector<std::string>& options, const std::string& question) {
    if (options.empty()) return "";

    int selected = 0;
    COORD startPos = getCursorPosition();

    auto render = [&]() {
        goToXY(0, startPos.Y);
        std::cout << ANSI::CLEAR_LINE << question << "\n";

        for (size_t i = 0; i < options.size(); ++i) {
            std::cout << ANSI::CLEAR_LINE;
            const std::string prefix = (selected == i) ? ANSI::CYAN + "> " + ANSI::RESET : "  ";
            std::cout << prefix << options[i] << "\n";
        }
        std::cout.flush();
    };

    render();

    while (true) {
        int key = _getch();

        if (key == 0 || key == 0xE0) {
            key = _getch();
            if (key == 72) {
                selected = (selected - 1 + options.size()) % options.size();
                render();
            } else if (key == 80) {
                selected = (selected + 1) % options.size();
                render();
            }
        } else if (key == ' ' || key == '\n' || key == '\r') {
            break;
        } else if (key == 3) {
            exit(0);
        }
    }

    goToXY(0, startPos.Y);

    size_t totalLines = options.size() + 1;
    for (size_t i = 0; i < totalLines; ++i) {
        goToXY(0, startPos.Y + i);
        std::cout << ANSI::CLEAR_LINE;
    }
    
    goToXY(0, startPos.Y); 

    return options[selected];
}