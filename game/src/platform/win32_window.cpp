#ifdef WIN32
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

    // Temporary disable resizing as larger windows make the game much easier to get high scores
    // Disable resizing
    DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
    // Center window
    RECT rc = {0, 0, width, height};
    AdjustWindowRect(&rc, style, FALSE);
    int windowWidth  = rc.right - rc.left;
    int windowHeight = rc.bottom - rc.top;

    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    hwnd = CreateWindowExA(
        0,
        "GameWindow",
        title,
        style, // WS_OVERLAPPEDWINDOW,
        x, y, // CW_USEDEFAULT, CW_USEDEFAULT`
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

void handleKeys(WPARAM wParam, bool down) {
    switch (wParam) {
        case VK_LEFT:
            Input::setKey(Key::Left, down);
            break;
        case VK_RIGHT:
            Input::setKey(Key::Right, down);
            break;
        case VK_UP:
            Input::setKey(Key::Up, down);
            break;
        case VK_DOWN:
            Input::setKey(Key::Down, down);
            break;
        case VK_RETURN:
            Input::setKey(Key::Enter, down);
            break;
        default:
            if (wParam == 'A') Input::setKey(Key::A, down);
            if (wParam == 'D') Input::setKey(Key::D, down);
            if (wParam == 'S') Input::setKey(Key::S, down);
            if (wParam == 'W') Input::setKey(Key::W, down);
            break;
    }
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            handleKeys(wParam, true);
            return 0;
        case WM_KEYUP:
            handleKeys(wParam, false);
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
#endif