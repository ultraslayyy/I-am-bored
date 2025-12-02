#include <iostream>
#include <map>
#include <cctype>

class Command {
public:
    virtual void execute() const = 0;
    virtual ~Command() {}
};

class AddCommand : public Command {
public:
    void execute() const override {
        std::cout << "Add command ran" << std::endl;
    }
};

void registerCommands(std::map<std::string, std::unique_ptr<Command>>& registry) {
    registry["add"] = std::make_unique<AddCommand>();
}

void toLowercase(std::string& s) {
    for (char& c : s) {
        c = std::tolower(c);
    }
}

int main(int argc, char* argv[]) {
    std::map<std::string, std::unique_ptr<Command>> commandRegistry;
    registerCommands(commandRegistry);

    if (argc < 2) {
        std::cerr << "Command is required" << std::endl;
        return 1;
    }

    std::string s = argv[1];

    toLowercase(s);

    auto it = commandRegistry.find(s);

    if (it == commandRegistry.end()) {
        std::cout << "Command not found: " << s << std::endl;
        return 1;
    }

    it->second->execute();
    return 0;
}