#ifdef DIRECT2D
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

    if (FAILED(factory->CreateHwndRenderTarget(props, hwndProps, &target))) return false;

    return true;
}

void D2DRenderer::clear(int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    target->BeginDraw();
    target->Clear(D2D1::ColorF(rf, gf, bf));
}

void D2DRenderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    D2D1_RECT_F rect = D2D1::RectF((FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h));

    ID2D1SolidColorBrush* brush = nullptr;
    target->CreateSolidColorBrush(D2D1::ColorF(rf, gf, bf), &brush);
    target->FillRectangle(&rect, brush);
    brush->Release();
}

void D2DRenderer::present() {
    target->EndDraw();
}

D2DRenderer::~D2DRenderer() {
    if (target) target->Release();
    if (factory) factory->Release();
}
#endif