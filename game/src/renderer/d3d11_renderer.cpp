#ifdef WIN32
#ifdef USE_DIRECT3D11
#include <cstring>
#include <string>
#include <d3dcompiler.h>
#include "d3d11_renderer.h"

#pragma comment(lib, "d3dcompiler")

// Vertex Shader
const char* vsSrc = R"(
struct VS_IN {
    float2 pos : POSITION;
    float4 col : COLOR;
};

struct PS_IN {
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_IN main(VS_IN input) {
    PS_IN output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.col = input.col;
    return output;
}
)";

// Pixel Shader
const char* psSrc = R"(
struct PS_IN {
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

float4 main(PS_IN input) : SV_Target {
    return input.col;
}
)";

bool D3D11Renderer::init(IWindow* window_p, int w, int h) {
    hwnd = (HWND)window_p->getNativeHandle();
    width = w;
    height = h;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &scd,
        &swapChain,
        &device,
        nullptr,
        &context
    );

    if (FAILED(hr)) return false;

    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();

    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);

    // Textures/shaders
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    hr = D3DCompile(vsSrc, strlen(vsSrc), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return false;
    }

    hr = D3DCompile(psSrc, strlen(psSrc), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return false;
    }

    if (errorBlob) {
        errorBlob->Release();
        errorBlob = nullptr;
    }

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    vsBlob->Release();
    psBlob->Release();

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(Vertex) * 6;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&bd, nullptr, &vertexBuffer);

    // Text
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);

    IDXGISurface* dxgiBackBuffer;
    swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&dxgiBackBuffer);
    
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    hr = d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiBackBuffer, &props, &d2dRenderTarget);
    dxgiBackBuffer->Release();

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFactory);

    return true;
}

void D3D11Renderer::clear(int r, int g, int b) {
    float colour[4] = {r / 255.0f, g / 255.0f, b / 255.0f, 1.0f};
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    context->ClearRenderTargetView(renderTargetView, colour);

    if (d2dRenderTarget) {
        d2dRenderTarget->BeginDraw();
    }
}

void D3D11Renderer::present() {
    if (d2dRenderTarget) {
        d2dRenderTarget->EndDraw();
    }

    swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING); // 0 = no vsync
}

void D3D11Renderer::resize(int w, int h) {
    width = w;
    height = h;

    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = nullptr;
    }

    swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();

    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);

    if (d2dRenderTarget) {
        d2dRenderTarget->Release();
        d2dRenderTarget = nullptr;
    }

    IDXGISurface* dxgiBackBuffer;
    swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&dxgiBackBuffer);

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiBackBuffer, &props, &d2dRenderTarget);
    dxgiBackBuffer->Release();
}

D3D11Renderer::~D3D11Renderer() {
    if (renderTargetView) renderTargetView->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();
    if (vertexBuffer) vertexBuffer->Release();
    if (vertexShader) vertexShader->Release();
    if (pixelShader) pixelShader->Release();
    if (inputLayout) inputLayout->Release();
    if (d2dRenderTarget) d2dRenderTarget->Release();
    if (d2dFactory) d2dFactory->Release();
    if (writeFactory) writeFactory->Release();
    if (textBrush) textBrush->Release();

    for (auto& [k, v] : textFormatCache) {
        if (v) v->Release();
    }
    textFormatCache.clear();
}

void D3D11Renderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float left   = (x / (float)width) * 2.0f - 1.0f;
    float right  = ((x + w) / (float)width) * 2.0f - 1.0f;
    float top    = 1.0f - (y / (float)height) * 2.0f;
    float bottom = 1.0f - ((y + h) / (float)height) * 2.0f;

    Vertex vertices[6] = {
        {left,  top,     rf, gf, bf, 1.0f},
        {right, bottom, rf, gf, bf, 1.0f},
        {left,  bottom,  rf, gf, bf, 1.0f},

        {left,  top,     rf, gf, bf, 1.0f},
        {right, top,    rf, gf, bf, 1.0f},
        {right, bottom, rf, gf, bf, 1.0f}
    };

    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vertices, sizeof(vertices));
    context->Unmap(vertexBuffer, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(inputLayout);

    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);

    context->Draw(6, 0);
}

IDWriteTextFormat* D3D11Renderer::getTextFormat(float size) {
    int key = (int)size;

    auto it = textFormatCache.find(key);
    if (it != textFormatCache.end()) {
        return it->second;
    }

    IDWriteTextFormat* format = nullptr;

    HRESULT hr = writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        size,
        L"en-us",
        &format
    );

    if (FAILED(hr)) {
        MessageBoxA(0, "CreateTextFormat failed", "DWrite", 0);
        return nullptr;
    }

    textFormatCache[key] = format;
    return format;
}

void D3D11Renderer::drawText(const char* text, float x, float y, float size, int r, int g, int b) {
    if (!d2dRenderTarget || !writeFactory) return;
    
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    std::wstring wtext(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, text, -1, &wtext[0], len);

    if (!textBrush) {
        d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rf, gf, bf, 1.0f), &textBrush);
    } else {
        textBrush->SetColor(D2D1::ColorF(rf, gf, bf, 1.0f));
    }

    IDWriteTextFormat* format = getTextFormat(size);
    if (!format) return;

    D2D1_RECT_F layoutRect = D2D1::RectF(x, y, (FLOAT)width, (FLOAT)height);

    d2dRenderTarget->DrawTextA(wtext.c_str(), (UINT32)wcslen(wtext.c_str()), format, &layoutRect, textBrush);
}
#endif
#endif