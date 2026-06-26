#ifdef WIN32
#ifdef USE_DIRECT3D12
#include "d3d12_renderer.h"
#include <d3dcompiler.h>
#include <string>

#define HR(x) do { HRESULT hr__ = (x); if (FAILED(hr__)) { __debugbreak(); } } while(0)

static const char* vsSrc = R"(
struct VSIn {
    float3 pos : POSITION;
    float3 col : COLOR;
};

struct PSIn {
    float4 pos : SV_POSITION;
    float3 col : COLOR;
};

PSIn main(VSIn input) {
    PSIn o;
    o.pos = float4(input.pos, 1.0);
    o.col = input.col;
    return o;
}
)";

static const char* psSrc = R"(
struct PSIn {
    float4 pos : SV_POSITION;
    float3 col : COLOR;
};

float4 main(PSIn input) : SV_TARGET {
    return float4(input.col, 1.0);
}
)";

bool D3D12Renderer::init(IWindow* window_p, int w, int h) {
    hwnd = (HWND)window_p->getNativeHandle();
    width = w;
    height = h;

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
            debug->EnableDebugLayer();
        }
    }
#endif

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
    allowTearing = FALSE;

    if (SUCCEEDED(factory.As(&factory5))) {
        BOOL tearing = FALSE;
        if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing)))) {
            allowTearing = tearing == TRUE;
        }
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) break;
    }

    if (!device) return false;

    D3D12_COMMAND_QUEUE_DESC qdesc = {};
    qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue));

    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.BufferCount = FrameCount;
    scd.Width = width;
    scd.Height = height;
    scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.SampleDesc.Count = 1;
    scd.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwap;
    factory->CreateSwapChainForHwnd(queue.Get(), hwnd, &scd, nullptr, nullptr, &tempSwap);
    tempSwap.As(&swapChain);

    frameIndex = swapChain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.NumDescriptors = FrameCount;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvHeap));
    
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (int i = 0; i < FrameCount; ++i) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));

        device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);

        handle.ptr += rtvDescriptorSize;
    }

    D3D11_CREATE_DEVICE_FLAG d3d11Flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0
    };

    Microsoft::WRL::ComPtr<ID3D11Device> baseDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> baseContext;

    HR(D3D11On12CreateDevice(device.Get(), d3d11Flags, featureLevels, _countof(featureLevels), (IUnknown**)&queue, 1, 0, &baseDevice, &baseContext, nullptr));

    HR(baseDevice.As(&d3d11Device));
    HR(baseContext.As(&d3d11Context));
    HR(d3d11Device.As(&d3d11On12Device));

    D3D11_RESOURCE_FLAGS flags = {};
    flags.BindFlags = D3D11_BIND_RENDER_TARGET;

    for (int i = 0; i < FrameCount; ++i) {
        HR(d3d11On12Device->CreateWrappedResource(backBuffers[i].Get(), &flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&wrappedBackBuffers[i])));
    }

    for (int i = 0; i < FrameCount; ++i) {
        HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frames[i].allocator)));
    }

    HR(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frames[0].allocator.Get(), nullptr, IID_PPV_ARGS(&cmd)));

    cmd->Close();

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    fenceValue = 1;

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    for (int i = 0; i < FrameCount; ++i) {
        frames[i].fenceValue = 0;
    }

    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_ROOT_PARAMETER rootParam = {};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParam.Constants.Num32BitValues = 1;
    rootParam.Constants.ShaderRegister = 0;
    rootParam.Constants.RegisterSpace = 0;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rsDesc.NumParameters = 1;
    rsDesc.pParameters = &rootParam;

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errBlob;

    if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob))) {
        if (errBlob) OutputDebugStringA((char*)errBlob->GetBufferPointer());
        return false;
    }

    device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig));

    Microsoft::WRL::ComPtr<ID3DBlob> vs, ps;

    D3DCompile(vsSrc, strlen(vsSrc), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vs, nullptr);
    D3DCompile(psSrc, strlen(psSrc), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &ps, nullptr);

    D3D12_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    // PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {layout, _countof(layout)};
    psoDesc.pRootSignature = rootSig.Get();
    psoDesc.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
    psoDesc.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};

    D3D12_RASTERIZER_DESC rasterizer = {};
    rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer.CullMode = D3D12_CULL_MODE_BACK;
    rasterizer.FrontCounterClockwise = FALSE;
    rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer.DepthClipEnable = TRUE;
    rasterizer.MultisampleEnable = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;
    rasterizer.ForcedSampleCount = 0;
    rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    psoDesc.RasterizerState = rasterizer;

    D3D12_BLEND_DESC blend = {};
    blend.AlphaToCoverageEnable = FALSE;
    blend.IndependentBlendEnable = FALSE;

    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };

    for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        blend.RenderTarget[i] = defaultRenderTargetBlendDesc;
    }

    psoDesc.BlendState = blend;

    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));

    ID3D12DescriptorHeap* heaps[] = { rtvHeap.Get() };
    cmd->SetDescriptorHeaps(1, heaps);


    UINT vbSize = sizeof(Vertex) * 6 * MaxRects;

    D3D12_HEAP_PROPERTIES heapUpload = {};
    heapUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapUpload.CreationNodeMask = 1;
    heapUpload.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC buffer = {};
    buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Alignment = 0;
    buffer.Width = vbSize;
    buffer.Height = 1;
    buffer.DepthOrArraySize = 1;
    buffer.MipLevels = 1;
    buffer.Format = DXGI_FORMAT_UNKNOWN;
    buffer.SampleDesc.Count = 1;
    buffer.SampleDesc.Quality = 0;
    buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    buffer.Flags = D3D12_RESOURCE_FLAG_NONE;

    for (int i = 0; i < FrameCount; ++i) {
        HR(device->CreateCommittedResource(&heapUpload, D3D12_HEAP_FLAG_NONE, &buffer, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexUpload[i])));

        vbView[i].BufferLocation = vertexUpload[i]->GetGPUVirtualAddress();
        vbView[i].StrideInBytes = sizeof(Vertex);
        vbView[i].SizeInBytes = vbSize;

        HR(vertexUpload[i]->Map(0, nullptr, (void**)&mappedPtr[i]));
    }

    D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &d2dFactory));

    HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwriteFactory));

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    HR(d3d11Device.As(&dxgiDevice));

    HR(d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice));
    HR(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContext));

    return true;
}

void D3D12Renderer::beginD2D() {
    ID3D11Resource* wrapped = wrappedBackBuffers[frameIndex].Get();

    d3d11On12Device->AcquireWrappedResources(&wrapped, 1);

    Microsoft::WRL::ComPtr<IDXGISurface> surface;
    HR(wrapped->QueryInterface(IID_PPV_ARGS(&surface)));

    D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));

    d2dTargetBitmap.Reset();

    if (!d2dBitmaps[frameIndex]) {
        HR(d2dContext->CreateBitmapFromDxgiSurface(surface.Get(), &props, &d2dBitmaps[frameIndex]));
    }

    d2dTargetBitmap = d2dBitmaps[frameIndex];

    d2dContext->SetTarget(d2dTargetBitmap.Get());
    d2dContext->BeginDraw();
}

void D3D12Renderer::endD2D() {
    d2dContext->EndDraw();
    d2dContext->SetTarget(nullptr);

    ID3D11Resource* wrapped = wrappedBackBuffers[frameIndex].Get();
    d3d11On12Device->ReleaseWrappedResources(&wrapped, 1);

    d3d11Context->Flush();
    d3d11Context->ClearState();
}

void D3D12Renderer::clear(int r, int g, int b) {
    float color[] = {r / 255.0f, g / 255.0f, b / 255.0f, 1.0f}; // force red

    currentVertexOffset = 0;

    FrameContext& frame = frames[frameIndex];

    if (fence->GetCompletedValue() < frame.fenceValue) {
        fence->SetEventOnCompletion(frame.fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    HR(frame.allocator->Reset());
    HR(cmd->Reset(frame.allocator.Get(), pso.Get()));
    cmd->SetPipelineState(pso.Get());

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = backBuffers[frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmd->ResourceBarrier(1, &barrier);

    beginD2D();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += frameIndex * rtvDescriptorSize;

    cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    cmd->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

    D3D12_VIEWPORT vp = {};
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MaxDepth = 1.0f;
    vp.MinDepth = 0.0f;

    D3D12_RECT scissor = {0, 0, width, height};

    cmd->RSSetViewports(1, &vp);
    cmd->RSSetScissorRects(1, &scissor);

    cmd->SetGraphicsRootSignature(rootSig.Get());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->IASetVertexBuffers(0, 1, &vbView[frameIndex]);
}

void D3D12Renderer::present() {
    if (currentVertexOffset > 0) {
        cmd->DrawInstanced(currentVertexOffset, 1, 0, 0);
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = backBuffers[frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmd->ResourceBarrier(1, &barrier);
    HR(cmd->Close());

    ID3D11Resource* release[] = { wrappedBackBuffers[frameIndex].Get() };
    d3d11On12Device->ReleaseWrappedResources(release, 1);
    d3d11Context->Flush();
    d3d11Context->ClearState();

    endD2D();

    ID3D12CommandList* lists[] = { cmd.Get() };
    queue->ExecuteCommandLists(1, lists);

    HR(swapChain->Present(0, allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0));

    FrameContext& frame = frames[frameIndex];
    const UINT64 fenceToWaitFor = fenceValue;

    HR(queue->Signal(fence.Get(), fenceToWaitFor));
    frame.fenceValue = fenceToWaitFor;
    fenceValue++;

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void D3D12Renderer::waitForGPU() {
    const UINT64 value = fenceValue;
    queue->Signal(fence.Get(), value);
    fenceValue++;

    if (fence->GetCompletedValue() < value) {
        fence->SetEventOnCompletion(value, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void D3D12Renderer::resize(int w, int h) {
    width = w;
    height = h;

    waitForGPU();

    for (int i = 0; i < FrameCount; ++i) {
        backBuffers[i].Reset();
    }

    swapChain->ResizeBuffers(FrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

    frameIndex = swapChain->GetCurrentBackBufferIndex();

    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (int i = 0; i < FrameCount; ++i) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
        device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
        handle.ptr += rtvDescriptorSize;
    }

    for (auto& b : d2dBitmaps) b.Reset();
}

D3D12Renderer::~D3D12Renderer() {
    waitForGPU();

    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
}

void D3D12Renderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    if (currentVertexOffset + 6 > MaxRects * 6) return; // TODO: flush + draw first

    float nx = (float)x / width * 2.0f - 1.0f;
    float ny = 1.0f - (float)y / height * 2.0f;
    float nx2 = (float)(x + w) / width * 2.0f - 1.0f;
    float ny2 = 1.0f - (float)(y + h) / height * 2.0f;

    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    Vertex quad[6] = {
        {nx,  ny,  0, rf, gf, bf},
        {nx2, ny,  0, rf, gf, bf},
        {nx,  ny2, 0, rf, gf, bf},

        {nx2, ny,  0, rf, gf, bf},
        {nx2, ny2, 0, rf, gf, bf},
        {nx,  ny2, 0, rf, gf, bf},
    };

    Vertex* dst = mappedPtr[frameIndex] + currentVertexOffset;
    memcpy(dst, quad, sizeof(quad));

    currentVertexOffset += 6;
}

void D3D12Renderer::drawText(const char* text, float x, float y, float size, int r, int g, int b) {
    if (!d2dContext) return;

    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    if (!textBrush) {
        d2dContext->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1, 1), &textBrush);
    }
    textBrush->SetColor(D2D1::ColorF(rf, gf, bf, 1.0f));

    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    std::wstring wtext(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext.data(), len);
    wtext.pop_back();

    Microsoft::WRL::ComPtr<IDWriteTextFormat> format;

    int key = (int)(size * 10.0f);

    auto it = textCache.find(key);
    if (it != textCache.end()) {
        format = it->second;
    } else {
        dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-us",
            &format
        );

        textCache[key] = format;
    }

    D2D1_RECT_F rect = D2D1::RectF(x, y, x + 1000, y + 1000);

    d2dContext->DrawTextA(wtext.c_str(), (UINT32)wtext.size(), format.Get(), &rect, textBrush.Get());
}
#endif
#endif