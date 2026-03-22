#pragma once
#include "../platform/win32_window.h"

class Application {
public:
    bool init();
    void run();

private:
    void update();
    void render();

    Win32Window window;
    float x = 250.0f;
};