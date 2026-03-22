#pragma once
#include <windows.h>

class Renderer {
public:
    bool init(HWND hwnd, int width, int height);

    void clear(int r, int g, int b);
    void drawRect(int x, int y, int w, int h, int r, int g, int b);
    void present();

private:
    HWND hwnd = nullptr;

    HDC backDC = nullptr;
    HBITMAP backBitmap = nullptr;

    int width = 0;
    int height = 0;
};