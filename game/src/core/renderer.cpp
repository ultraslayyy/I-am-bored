#include "renderer.h"

bool Renderer::init(HWND hwnd, int w, int h) {
    this->hwnd = hwnd;
    this->width = w;
    this->height = h;

    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(hwnd, size);

    if (FAILED(pFactory->CreateHwndRenderTarget(props, hwndProps, &pRenderTarget))) return false;

    return true;
}

void Renderer::clear(int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(rf, gf, bf));
}

void Renderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    D2D1_RECT_F rect = D2D1::RectF((FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h));

    ID2D1SolidColorBrush* brush = nullptr;
    pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rf, gf, bf), &brush);
    pRenderTarget->FillRectangle(&rect, brush);
    brush->Release();
}

void Renderer::present() {
    pRenderTarget->EndDraw();
}

Renderer::~Renderer() {
    if (pRenderTarget) pRenderTarget->Release();
    if (pFactory) pFactory->Release();
}