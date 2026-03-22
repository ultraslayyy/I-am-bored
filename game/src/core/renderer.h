#pragma once
#include <d2d1.h>

class Renderer {
public:
    bool init(HWND hwnd, int width, int height);

    void clear(int r, int g, int b);
    void drawRect(int x, int y, int w, int h, int r, int g, int b);
    void present();
    ~Renderer();

private:
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
};