#ifdef GDI
#include "gdi_renderer.h"

bool GDIRenderer::init(HWND hwnd, int w, int h) {
    this->hwnd = hwnd;
    this->width = w;
    this->height = h;

    HDC hdc = GetDC(hwnd);

    backDC = CreateCompatibleDC(hdc);
    backBitmap = CreateCompatibleBitmap(hdc, width, height);
    oldBitmap = (HBITMAP)SelectObject(backDC, backBitmap);
    SelectObject(backDC, backBitmap);

    ReleaseDC(hwnd, hdc);

    return true;
}

void GDIRenderer::clear(int r, int g, int b) {
    HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
    RECT rect = { 0, 0, width, height };
    FillRect(backDC, &rect, brush);
    DeleteObject(brush);
}

void GDIRenderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
    RECT rect = { x, y, x + w, y + h };
    FillRect(backDC, &rect, brush);
    DeleteObject(brush);
}

void GDIRenderer::present() {
    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, 0, 0, width, height, backDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

GDIRenderer::~GDIRenderer() {
    if (backDC && oldBitmap) {
        SelectObject(backDC, oldBitmap);
    }

    if (backBitmap) {
        DeleteObject(backBitmap);
        backBitmap = nullptr;
    }

    if (backDC) {
        DeleteObject(backDC);
        backDC = nullptr;
    }
}
#endif