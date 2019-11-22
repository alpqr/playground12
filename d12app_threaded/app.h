#ifndef APP_H
#define APP_H

#include "common.h"
#include "descheapmgr.h"
#include "timestamp.h"
#include "builder.h"

struct App
{
    App(HINSTANCE hInstance, HWND hWnd, Builder::Type builderType = Builder::Type::Threaded);
    ~App();

    bool initialize();
    void releaseResources();
    void waitGpu();
    void render();
    void resize(UINT newWidth, UINT newHeight);
    void logVidMemUsage();

    void bumpFrameFence();
    void waitForFrameFence();
    bool createSwapchainViews();
    void releaseSwapchainViews();
    void handleLostDevice();
    void beginFrame();
    void endFrame();

    void requestUpdate() { m_needsRender = true; }
    void maybeUpdate() { if (m_needsRender) render(); }

    Builder::Type builderType() const { return m_builderType; }
    void addBuilder(Builder *b);
    void addBuilders(std::initializer_list<Builder *> args);
    void deleteBuilder(Builder *b);

    void postToAllBuildersAndWait(Builder::Event e);

    using FrameFunc = std::function<void()>;
    void addPreFrameFunc(FrameFunc f) { m_preFrameFuncs.push_back(f); }
    void addPostFrameFunc(FrameFunc f) { m_postFrameFuncs.push_back(f); }

    HINSTANCE m_hInstance;
    HWND m_hWnd;
    Builder::Type m_builderType;
    UINT m_width = DEFAULT_WIDTH;
    UINT m_height = DEFAULT_HEIGHT;
    bool m_zeroSize = false;
    IDXGIFactory3 *m_dxgiFactory = nullptr;
    IDXGIAdapter3 *m_adapter = nullptr;
    ID3D12Device *m_device = nullptr;
    D3D12_FEATURE_DATA_D3D12_OPTIONS m_features = {};
    D3D12_FEATURE_DATA_ARCHITECTURE m_archFeatures = {};
    ID3D12CommandQueue *m_cmdQueue = nullptr;
    IDXGISwapChain3 *m_swapchain = nullptr;
    UINT m_currentFrameSlot; // 0..FRAMES_IN_FLIGHT-1
    ID3D12Fence *m_frameFence = nullptr;
    UINT64 m_lastFrameFenceValue = 0;
    UINT64 m_frameFenceValues[SWAPCHAIN_BUFFER_COUNT] = {};
    HANDLE m_frameFenceEvent = nullptr;
    DescHeapMgr m_descHeapMgr;
    ID3D12Resource *m_rt[SWAPCHAIN_BUFFER_COUNT] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtv[SWAPCHAIN_BUFFER_COUNT] = {};
    ID3D12Resource *m_ds = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsv = {};
    ID3D12CommandAllocator *m_cmdAllocator[FRAMES_IN_FLIGHT] = {};
    ID3D12GraphicsCommandList *m_mainThreadDrawCmdList[2] = {};
    bool m_needsRender = false;
    Timestamp m_renderTimestamp;
    std::vector<Builder *> m_builders;
    std::vector<HANDLE> m_waitEvents;
    std::vector<ID3D12CommandList *> m_cmdListBatch;
    std::vector<FrameFunc> m_preFrameFuncs;
    std::vector<FrameFunc> m_postFrameFuncs;
};

#endif
