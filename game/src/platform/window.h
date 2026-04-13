#pragma once
#include <functional>

class IWindow {
public:
    virtual bool create(int width, int height, const char* title) = 0;
    virtual void pollEvents() = 0;
    bool isRunning() const {
        return running;
    }
    void setRunning(bool val) {
        running = val;
    }

    virtual void* getNativeHandle() const = 0;
    virtual void* getNativeDisplay() const = 0;

    std::function<void(int, int)> onResize;

private:
    bool running = true;
};