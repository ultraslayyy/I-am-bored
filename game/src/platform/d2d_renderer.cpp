#ifdef USE_DIRECT2D
#include <string>
#include "d2d_renderer.h"

bool D2DRenderer::init(HWND hwnd, int w, int h) {
    this->hwnd = hwnd;
    this->width = w;
    this->height = h;

    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))) return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(hwnd, size);

    if (FAILED(factory->CreateHwndRenderTarget(props, hwndProps, &pRenderTarget))) return false;

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&writeFactory));
    writeFactory->CreateTextFormat(
        L"Segoe UI",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        24.0f,
        L"en-us",
        &textFormat
    );

    return true;
}

void D2DRenderer::clear(int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(rf, gf, bf));
}

void D2DRenderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    D2D1_RECT_F rect = D2D1::RectF((FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h));

    ID2D1SolidColorBrush* brush = nullptr;
    pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rf, gf, bf), &brush);
    pRenderTarget->FillRectangle(&rect, brush);
    brush->Release();
}

void D2DRenderer::drawText(const char* text, float x, float y, float size, int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    std::wstring wtext(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, text, -1, &wtext[0], len);
    
    ID2D1SolidColorBrush* brush = nullptr;
    pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rf, gf, bf), &brush);

    IDWriteTextFormat* format = nullptr;
    writeFactory->CreateTextFormat(
        L"Segoe UI",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        size,
        L"en-us",
        &format
    );

    D2D1_RECT_F layoutRect = D2D1::RectF(x, y, (FLOAT)width, (FLOAT)height);

    pRenderTarget->DrawTextA(wtext.c_str(), (UINT32)wcslen(wtext.c_str()), format, &layoutRect, brush);

    format->Release();
    brush->Release();
}

void D2DRenderer::present() {
    pRenderTarget->EndDraw();
}

D2DRenderer::~D2DRenderer() {
    if (pRenderTarget) pRenderTarget->Release();
    if (factory) factory->Release();
    if (textFormat) textFormat->Release();
    if (writeFactory) writeFactory->Release();
}
#endif