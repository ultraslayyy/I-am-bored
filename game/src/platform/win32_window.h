#pragma once
#include <windows.h>

class Win32Window {
public:
    bool create(int width, int height, const char *title);
    void pollEvents();
    bool isRunning() const;

    HWND getHandle() const { return hwnd; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd = nullptr;
    bool running = true;
};