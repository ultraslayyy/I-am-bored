#pragma once
#ifdef WIN32
#include "../platform/win32_window.h"
#elif HAS_X11
#include "../platform/x11_window.h"
#endif

class Application {
public:
    bool init();
    void run();

private:
#ifdef WIN32
    Win32Window window;
#elif HAS_X11
    X11Window window;
#endif
};