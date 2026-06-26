#pragma once
#ifdef WIN32
#include <d3d11.h>
#include <d3d11on12.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <unordered_map>
#include <cstdint>
#include "../core/renderer.h"

class D3D12Renderer : public IRenderer {
public:
    bool init(IWindow* window, int width, int height) override;
    void clear(int r, int g, int b) override;
    void present() override;
    void resize(int w, int h) override;
    void drawRect(int x, int y, int w, int h, int r, int g, int b) override;
    void drawText(const char* text, float x, float y, float size, int r, int g, int b) override;

    ~D3D12Renderer();

    int getWidth() const override { return width; }
    int getHeight() const override { return height; }

private:
    void waitForGPU();

    void beginD2D();
    void endD2D();

    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    static constexpr int FrameCount = 2;
    static constexpr int MaxRects = 1000;

    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd;

    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
    UINT rtvDescriptorSize = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers[FrameCount];

    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    UINT64 fenceValue = 0;

    UINT frameIndex = 0;

    struct Vertex {
        float x, y, z;
        float r, g, b;
    };

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexUpload[FrameCount];
    Vertex* mappedPtr[FrameCount] = {};
    D3D12_VERTEX_BUFFER_VIEW vbView[FrameCount] = {};

    UINT currentVertexOffset = 0;

    struct FrameContext {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        UINT64 fenceValue;
    };
    FrameContext frames[FrameCount];

    BOOL allowTearing = FALSE;

    Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context;
    Microsoft::WRL::ComPtr<ID3D11On12Device> d3d11On12Device;

    Microsoft::WRL::ComPtr<ID3D11Resource> wrappedBackBuffers[FrameCount];

    Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dContext;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTargetBitmap;

    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush;

    std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<IDWriteTextFormat>> textCache;

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmaps[FrameCount];
};
#endif