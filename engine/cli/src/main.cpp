#include "cli.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>

struct ProjectConfig {
    std::string name;
    std::string language;
};

ProjectConfig loadConfig() {
    std::ifstream file("engine.json");
    ProjectConfig config;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (content.find("\"language\": \"csharp\"") != std::string::npos) {
        config.language = "csharp";
    } else {
        config.language = "cpp";
    }

    if (content.find("\"name\":") != std::string::npos) {
        auto start = content.find("\"name\":") + 8;
        auto end = content.find("\"", start);
        config.name = content.substr(start, end - start);
    }

    return config;
}

void createProject(const std::string& name, const std::string& lang) {
    std::filesystem::create_directory(name);

    if (lang == "cpp") {
        std::filesystem::copy("templates/cpp", name, std::filesystem::copy_options::recursive);
    } else {
        std::filesystem::copy("templates/csharp", name, std::filesystem::copy_options::recursive);
    }

    std::ofstream config(name + "/engine.json");
    config << "{\n";
    config << "  \"name\": \"" << name << "\",\n";
    config << "  \"language\": \"" << lang << "\",\n";
    config << "}\n";

    std::cout << "Created " << lang << " project: " << name << "\n";
}

void buildProject() {
    auto config = loadConfig();

    if (config.language == "cpp") {
        system("cmake -B build -S .");
        system("cmake --build build");
    } else if (config.language == "csharp") {
        system("dotnet build");
    }
}

void runProject() {
    auto config = loadConfig();

    if (config.language == "cpp") {
        std::string cmd = "cmake -B build -S . && cmake --build build && ./build/" + config.name;
        system(cmd.c_str());
    } else if (config.language == "csharp") {
        system("dotnet run");
    }
}

void cleanProject() {
    auto config = loadConfig();

    if (config.language == "cpp") {
        std::filesystem::remove_all("build");
    } else if (config.language == "csharp") {
        system("dotnet clean");
    }

    std::cout << "Cleaned project\n";
}

bool checkCommand(const std::string& cmd) {
    std::string test = "where " + cmd + " >nul 2>&1";
    return system(test.c_str()) == 0;
}

void doctor() {
    std::cout << "Running diagnostics...\n\n";

    std::cout << "CMake: " << (checkCommand("cmake") ? "OK" : "MISSING") << "\n";
    std::cout << "C++ Compiler: " << (checkCommand("g++") || checkCommand("clang++") || checkCommand("cl.exe") ? "OK" : "MISSING") << "\n";
    std::cout << ".NET: " << (checkCommand("dotnet") ? "OK" : "MISSING") << "\n";

    std::cout << "\nDone.\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Commands: new, build, run, clean, doctor\n";
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "new") {
        if (argc < 3) {
            std::cout << "Usage: engine new <name> [cpp|csharp]\n";
            return 0;
        }

        std::string lang = (argc >= 4) ? argv[3] : "cpp";
        createProject(argv[2], lang);
    } else if (cmd == "build") {
        buildProject();
    } else if (cmd == "run") {
        runProject();
    } else if (cmd == "clean") {
        cleanProject();
    } else if (cmd == "doctor") {
        doctor();
    } else {
        std::cout << "Unknown command\n";
    }
}