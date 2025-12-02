#include <chrono>
#include <iostream>

class ScopeTimer {
private:
    std::chrono::high_resolution_clock::time_point startTime;

public:
    ScopeTimer() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    ~ScopeTimer() {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = now - startTime;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::cout << "Time in scope: " << ms << "ms" << std::endl;
    }
};

int main() {
    if (true) {
        ScopeTimer t;

        for (int i = 0; i < 10000000; i++)
        {
            continue;
        }
    }

    std::cout << "Exited if" << std::endl;

    return 0;
}