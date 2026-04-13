#pragma once
#ifdef WIN32
#include <functional>
#include <windows.h>
#include "window.h"

class Win32Window : public IWindow {
public:
    bool create(int width, int height, const char *title) override;
    void pollEvents() override;

    void* getNativeHandle() const override { return (void*)hwnd; }
    void* getNativeDisplay() const override { return nullptr; }

    HWND getHandle() const { return hwnd; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd = nullptr;
    bool running = true;
};
#endif