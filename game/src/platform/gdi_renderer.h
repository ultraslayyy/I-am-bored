#pragma once
#include <windows.h>
#include "../core/renderer.h"

class GDIRenderer : public IRenderer {
public:
    bool init(HWND hwnd, int width, int height) override;

    void clear(int r, int g, int b) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void present() override;

    ~GDIRenderer();

private:
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    HDC backDC = nullptr;
    HBITMAP backBitmap = nullptr;
    HBITMAP oldBitmap = nullptr;
};