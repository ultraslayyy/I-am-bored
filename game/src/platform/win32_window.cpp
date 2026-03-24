#include "../core/input.h"
#include "win32_window.h"

static Win32Window* g_window = nullptr;

bool Win32Window::create(int width, int height, const char *title) {
    g_window = this;

    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "GameWindow";

    RegisterClass(&wc);

    hwnd = CreateWindowExA(
        0,
        "GameWindow",
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL, NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    return hwnd != nullptr;
}

void Win32Window::pollEvents() {
    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            running = false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Win32Window::isRunning() const {
    return running;
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_LEFT) Input::setKey(Key::Left, true);
            if (wParam == VK_RIGHT) Input::setKey(Key::Right, true);
            if (wParam == VK_UP) Input::setKey(Key::Up, true);
            if (wParam == VK_DOWN) Input::setKey(Key::Down, true);
            return 0;
        case WM_KEYUP:
            if (wParam == VK_LEFT) Input::setKey(Key::Left, false);
            if (wParam == VK_RIGHT) Input::setKey(Key::Right, false);
            if (wParam == VK_UP) Input::setKey(Key::Up, false);
            if (wParam == VK_DOWN) Input::setKey(Key::Down, false);
            return 0;
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);

            if (g_window && g_window->onResize) {
                g_window->onResize(w, h);
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}