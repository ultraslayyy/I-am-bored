#ifdef USE_GDI
#include <string>
#include "gdi_renderer.h"

bool GDIRenderer::init(HWND hwnd, int w, int h) {
    this->hwnd = hwnd;
    this->width = w;
    this->height = h;

    HDC hdc = GetDC(hwnd);

    backDC = CreateCompatibleDC(hdc);
    backBitmap = CreateCompatibleBitmap(hdc, width, height);
    oldBitmap = (HBITMAP)SelectObject(backDC, backBitmap);

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

void GDIRenderer::drawText(const char* text, float x, float y, float size, int r, int g, int b) {
    if (!backDC) return;

    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    std::wstring wtext(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, text, -1, &wtext[0], len);

    HFONT font = CreateFontW(
        -(int)size,
        0,
        0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );

    HFONT oldFont = (HFONT)SelectObject(backDC, font);

    SetTextColor(backDC, RGB(r, g, b));

    SetBkMode(backDC, TRANSPARENT);

    TextOutW(backDC, (int)x, (int)y, wtext.c_str(), (int)wcslen(wtext.c_str()));

    SelectObject(backDC, oldFont);
    DeleteObject(font);
}

void GDIRenderer::present() {
    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, 0, 0, width, height, backDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void GDIRenderer::resize(int w, int h) {
    width = w;
    height = h;

    if (backDC && oldBitmap) {
        SelectObject(backDC, oldBitmap);
    }
    if (backBitmap) {
        DeleteObject(backBitmap);
        backBitmap = nullptr;
    }

    HDC hdc = GetDC(hwnd);
    backDC = CreateCompatibleDC(hdc);
    backBitmap = CreateCompatibleBitmap(hdc, width, height);
    oldBitmap = (HBITMAP)SelectObject(backDC, backBitmap);
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