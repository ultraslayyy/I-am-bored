#pragma once
#ifdef WIN32
#include <d2d1.h>
#include <dwrite.h>
#include "../core/renderer.h"

#pragma comment(lib, "dwrite")

class D2DRenderer : public IRenderer {
public:
    bool init(IWindow* window, int width, int height) override;

    void clear(int r, int g, int b) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void drawText(const char* text, float x, float y, float size, int r, int g, int b) override;
    void present() override;

    void resize(int w, int h) override;
    int getWidth() const override { return width; }
    int getHeight() const override { return height; }

    ~D2DRenderer();

private:
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    ID2D1Factory* factory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    IDWriteFactory* writeFactory = nullptr;
    IDWriteTextFormat* textFormat = nullptr;
};
#endif