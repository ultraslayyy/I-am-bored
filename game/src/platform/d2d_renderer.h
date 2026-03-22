#pragma once
#include <d2d1.h>
#include "../core/renderer.h"

class D2DRenderer : public IRenderer {
public:
    bool init(HWND hwnd, int width, int height) override;

    void clear(int r, int g, int b) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void present() override;

    ~D2DRenderer();

private:
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    ID2D1Factory* factory = nullptr;
    ID2D1HwndRenderTarget* target = nullptr;
};