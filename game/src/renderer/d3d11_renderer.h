#pragma once
#ifdef WIN32
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11.h>
#include <dxgi.h>
#include <unordered_map>
#include "../core/renderer.h"

#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dwrite")

class D3D11Renderer : public IRenderer {
public:
    bool init(IWindow* window, int width, int height) override;
    void clear(int r, int g, int b) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void drawText(const char* text, float x, float y, float size, int r, int g, int b) override;
    void present() override;
    void resize(int w, int h) override;

    ~D3D11Renderer();

    int getWidth() const override { return width; }
    int getHeight() const override { return height; }

private:
    IDWriteTextFormat* getTextFormat(float size);

    HWND hwnd;

    int width = 0;
    int height = 0;

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;

    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;

    ID2D1Factory* d2dFactory = nullptr;
    ID2D1RenderTarget* d2dRenderTarget = nullptr;
    IDWriteFactory* writeFactory = nullptr;
    ID2D1SolidColorBrush* textBrush = nullptr;

    struct Vertex {
        float x, y;
        float r, g, b, a;
    };

    std::unordered_map<int, IDWriteTextFormat*> textFormatCache;
};
#endif