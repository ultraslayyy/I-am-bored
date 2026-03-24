#pragma once
#include <GL/gl.h>
#include "../core/renderer.h"

class GLRenderer : public IRenderer {
public:
    bool init(HWND hwnd, int width, int height) override;

    void clear(int r, int g, int b) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void drawText(const char* text, float x, float y, float size, int r, int g, int b) override;
    void present() override;

    void resize(int w, int h) override;
    int getWidth() const override { return width; }
    int getHeight() const override { return height; }

    ~GLRenderer();

private:
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;

    GLuint fontBase = 0;

    int width = 0;
    int height = 0;
};