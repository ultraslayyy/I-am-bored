#include <string>
#include <vector>
#include <windows.h>

class CLI {
public:
    CLI();

    std::string MultipleChoice(const std::vector<std::string>& options, const std::string& question);
    void goToXY(int x, int y);
    COORD getCursorPosition();

private:
    HANDLE hConsole;
};

namespace ANSI {
    const std::string RESET = "\x1b[0m";
    const std::string GREY = "\x1b[90m";
    const std::string CYAN = "\x1b[36m";
    const std::string GREEN = "\x1b[32m";
    const std::string RED = "\x1b[31m";
    const std::string YELLOW = "\x1b[33m";
    const std::string MAGENTA = "\x1b[35m";
    const std::string CLEAR_LINE = "\x1b[2K";
}