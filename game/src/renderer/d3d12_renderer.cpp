#ifdef WIN32
#ifdef USE_DIRECT3D12
#include "d3d12_renderer.h"
#include <d3dcompiler.h>

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

    for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
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

    return true;
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
}

D3D12Renderer::~D3D12Renderer() {
    waitForGPU();

    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
}

void D3D12Renderer::drawRect(int x, int y, int w, int h, int r, int g, int b) {
    if (currentVertexOffset + 6 > MaxRects * 6) return; // TODO: Draw and keep going, check D3D11

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
#endif
#endif