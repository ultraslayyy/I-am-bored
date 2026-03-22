#pragma once
#include "../platform/win32_window.h"

class Application {
public:
    bool init();
    void run();

private:
    Win32Window window;
};