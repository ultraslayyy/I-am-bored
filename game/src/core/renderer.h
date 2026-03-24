#pragma once
#include <windows.h>

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool init(HWND hwnd, int width, int height) = 0;

    virtual void clear(int r, int g, int b) = 0;
    virtual void drawRect(int x, int y, int w, int h, int r, int g, int b) = 0;
    virtual void drawText(const char* text, float x, float y, float size, int r, int g, int b) = 0;
    virtual void present() = 0;

    virtual void resize(int w, int h) = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
};