#include "app.h"
#include "res.h"

void App::logVidMemUsage()
{
    DXGI_QUERY_VIDEO_MEMORY_INFO vidMemInfo;
    if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vidMemInfo))) {
        log("Video memory local: Budget %llu KB CurrentUsage %llu KB AvailableForReservation %llu KB CurrentReservation %llu KB",
            vidMemInfo.Budget / 1024, vidMemInfo.CurrentUsage / 1024,
            vidMemInfo.AvailableForReservation / 1024, vidMemInfo.CurrentReservation / 1024);
    }
    if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &vidMemInfo))) {
        log("Video memory non-local: Budget %llu KB CurrentUsage %llu KB AvailableForReservation %llu KB CurrentReservation %llu KB",
            vidMemInfo.Budget / 1024, vidMemInfo.CurrentUsage / 1024,
            vidMemInfo.AvailableForReservation / 1024, vidMemInfo.CurrentReservation / 1024);
    }
}

void App::bumpFrameFence()
{
    m_lastFrameFenceValue += 1;
    m_frameFenceValues[m_currentFrameSlot] = m_lastFrameFenceValue;
    m_cmdQueue->Signal(m_frameFence, m_frameFenceValues[m_currentFrameSlot]);
}

void App::waitForFrameFence()
{
    if (m_frameFence->GetCompletedValue() < m_frameFenceValues[m_currentFrameSlot]) {
        m_frameFence->SetEventOnCompletion(m_frameFenceValues[m_currentFrameSlot], m_frameFenceEvent);
        WaitForSingleObject(m_frameFenceEvent, INFINITE);
    }
}

void App::waitGpu()
{
    if (!m_cmdQueue)
        return;

    bumpFrameFence();
    waitForFrameFence();
}

bool App::createSwapchainViews()
{
    D3D12_CPU_DESCRIPTOR_HANDLE firstRtv = m_descHeapMgr.allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SWAPCHAIN_BUFFER_COUNT);
    const UINT rtvStride = m_descHeapMgr.handleSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i) {
        HRESULT hr = m_swapchain->GetBuffer(i, IID_ID3D12Resource, reinterpret_cast<void **>(&m_rt[i]));
        if (FAILED(hr)) {
            logHr("Failed to get swapchain buffer", hr);
            return false;
        }
        m_rtv[i].ptr = firstRtv.ptr + SIZE_T(i) * rtvStride;
        m_device->CreateRenderTargetView(m_rt[i], nullptr, m_rtv[i]);
    }

    m_dsv = m_descHeapMgr.allocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    m_ds = Res::createDepthStencil(m_device, m_dsv, m_width, m_height, 1);
    if (!m_ds)
        return false;

    return true;
}

void App::releaseSwapchainViews()
{
    if (m_ds) {
        m_ds->Release();
        m_ds = nullptr;
    }
    if (m_dsv.ptr) {
        m_descHeapMgr.release(m_dsv, 1);
        m_dsv.ptr = 0;
    }
    for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i) {
        if (m_rt[i]) {
            m_rt[i]->Release();
            m_rt[i] = nullptr;
        }
        if (m_rtv[i].ptr) {
            m_descHeapMgr.release(m_rtv[i], 1);
            m_rtv[i].ptr = 0;
        }
    }
}

bool App::initialize()
{
    log("App::initialize() Threaded command list building: %s Forced adapter index: %d Sync interval: %d Debug layer: %s",
        m_threadModel == Builder::ThreadModel::Threaded ? "yes" : "no",
        ADAPTER_INDEX, PRESENT_SYNC_INTERVAL, ENABLE_DEBUG_LAYER ? "yes" : "no");

    HRESULT hr = CreateDXGIFactory2(0, IID_IDXGIFactory2, reinterpret_cast<void **>(&m_dxgiFactory));
    if (FAILED(hr)) {
        logHr("Failed to create DXGI factory", hr);
        return false;
    }

    IDXGIAdapter1 *adapterToUse = nullptr;
    IDXGIAdapter1 *adapter;

    for (int adapterIndex = 0; m_dxgiFactory->EnumAdapters1(UINT(adapterIndex), &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        log("Adapter %d: '%ls' (vendor 0x%X device 0x%X flags 0x%X)",
            adapterIndex, desc.Description, desc.VendorId, desc.DeviceId, desc.Flags);
        if (!adapterToUse && (ADAPTER_INDEX < 0 || ADAPTER_INDEX == adapterIndex)) {
            adapterToUse = adapter;
            log("  using this adapter");
        } else {
            adapter->Release();
        }
    }
    if (!adapterToUse) {
        log("No adapter");
        return false;
    }

    if (FAILED(adapterToUse->QueryInterface(IID_IDXGIAdapter3, reinterpret_cast<void **>(&m_adapter)))) {
        log("IDXGIAdapter3 not supported");
        adapterToUse->Release();
        return false;
    }

    if (ENABLE_DEBUG_LAYER) {
        ID3D12Debug *debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_ID3D12Debug, reinterpret_cast<void **>(&debugController)))) {
            debugController->EnableDebugLayer();
            log("D3D12 debug layer enabled");
            debugController->Release();
        }
    }

    hr = D3D12CreateDevice(adapterToUse, FEATURE_LEVEL, IID_ID3D12Device, reinterpret_cast<void **>(&m_device));
    adapterToUse->Release();
    if (FAILED(hr)) {
        logHr("Failed to create D3D12 device", hr);
        return false;
    }

    hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_features, sizeof(m_features));
    if (FAILED(hr)) {
        logHr("Failed to query device options", hr);
        return false;
    }
    hr = m_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &m_archFeatures, sizeof(m_archFeatures));
    if (FAILED(hr)) {
        logHr("Failed to query arch features", hr);
        return false;
    }
    log("Resource binding tier: %d Resource heap tier: %d Tile-based: %d UMA: %d CacheCoherentUMA: %d",
        m_features.ResourceBindingTier, m_features.ResourceHeapTier,
        m_archFeatures.TileBasedRenderer, m_archFeatures.UMA, m_archFeatures.CacheCoherentUMA);

    logVidMemUsage();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_ID3D12CommandQueue, reinterpret_cast<void **>(&m_cmdQueue)))) {
        logHr("Failed to create command queue", hr);
        return false;
    }

    IDXGISwapChain1 *swapchain1;
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = m_width;
    desc.Height = m_height;
    desc.Format = SWAPCHAIN_FORMAT;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = SWAPCHAIN_BUFFER_COUNT;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    hr = m_dxgiFactory->CreateSwapChainForHwnd(m_cmdQueue, m_hWnd, &desc, nullptr, nullptr, &swapchain1);
    if (FAILED(hr)) {
        logHr("Failed to create swapchain", hr);
        return false;
    }

    hr = swapchain1->QueryInterface(IID_IDXGISwapChain3, reinterpret_cast<void **>(&m_swapchain));
    swapchain1->Release();
    if (FAILED(hr)) {
        logHr("IDXGISwapChain3 not supported", hr);
        return false;
    }

    m_currentFrameSlot = m_swapchain->GetCurrentBackBufferIndex();

    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, reinterpret_cast<void **>(&m_frameFence));
    if (FAILED(hr)) {
        logHr("Failed to create fence", hr);
        return false;
    }
    m_frameFenceEvent = CreateEvent(nullptr, false, false, nullptr);
    for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i)
        m_frameFenceValues[i] = 0;

    m_dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

    m_descHeapMgr.initialize(m_device);

    createSwapchainViews();

    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_ID3D12CommandAllocator,
            reinterpret_cast<void **>(&m_cmdAllocator[i]));
        if (FAILED(hr)) {
            logHr("Failed to create command allocator", hr);
            return false;
        }
    }

    for (int i = 0; i < 2; ++i) {
        hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator[0], nullptr, IID_ID3D12GraphicsCommandList,
            reinterpret_cast<void **>(&m_mainThreadDrawCmdList[i]));
        if (FAILED(hr)) {
            logHr("Failed to create graphics command list", hr);
            return false;
        }
        m_mainThreadDrawCmdList[i]->Close();
    }

    return true;
}

void App::releaseResources()
{
    waitGpu();

    for (ReleaseResourcesFunc f : m_releaseResourcesFuncs)
        f();

    postToAllBuildersAndWait(Builder::Event::ReleaseResources);

    for (int i = 0; i < 2; ++i) {
        if (m_mainThreadDrawCmdList[i]) {
            m_mainThreadDrawCmdList[i]->Release();
            m_mainThreadDrawCmdList[i] = nullptr;
        }
    }

    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        if (m_cmdAllocator[i]) {
            m_cmdAllocator[i]->Release();
            m_cmdAllocator[i] = nullptr;
        }
    }

    releaseSwapchainViews();

    m_descHeapMgr.releaseResources();

    if (m_frameFence) {
        m_frameFence->Release();
        m_frameFence = nullptr;
    }

    if (m_frameFenceEvent) {
        CloseHandle(m_frameFenceEvent);
        m_frameFenceEvent = nullptr;
    }

    if (m_swapchain) {
        m_swapchain->Release();
        m_swapchain = nullptr;
    }

    if (m_cmdQueue) {
        m_cmdQueue->Release();
        m_cmdQueue = nullptr;
    }

    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }

    if (m_adapter) {
        m_adapter->Release();
        m_adapter = nullptr;
    }

    if (m_dxgiFactory) {
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
    }
}

void App::resize(UINT newWidth, UINT newHeight)
{
    if (m_width == newWidth && m_height == newHeight)
        return;

    m_width = newWidth;
    m_height = newHeight;
    m_zeroSize = m_width < 1 || m_height < 1; // f.ex. when minimized
    if (m_zeroSize)
        m_width = m_height = 1;

    log("resize %ux%u", m_width, m_height);

    if (m_swapchain) {
        waitGpu();
        releaseSwapchainViews();
        HRESULT hr = m_swapchain->ResizeBuffers(SWAPCHAIN_BUFFER_COUNT, m_width, m_height, SWAPCHAIN_FORMAT, 0);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            handleLostDevice();
            return;
        } else if (FAILED(hr)) {
            logHr("Failed to resize swapchain buffer", hr);
        }
        createSwapchainViews();
        m_currentFrameSlot = m_swapchain->GetCurrentBackBufferIndex();
    }
}

void App::handleLostDevice()
{
    releaseResources();
    requestUpdate();
}

void App::beginFrame()
{
    waitForFrameFence();

    m_cmdAllocator[m_currentFrameSlot]->Reset();

    for (FrameExtraFunc f : m_preFrameFuncs)
        f();

    m_mainThreadDrawCmdList[0]->Reset(m_cmdAllocator[m_currentFrameSlot], nullptr);
    D3D12_RESOURCE_BARRIER rtBarrier = {};
    rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rtBarrier.Transition.pResource = m_rt[m_currentFrameSlot];
    rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_mainThreadDrawCmdList[0]->ResourceBarrier(1, &rtBarrier);
    m_mainThreadDrawCmdList[0]->Close();
}

void App::endFrame(const BuilderTable *bldTab)
{
    m_mainThreadDrawCmdList[1]->Reset(m_cmdAllocator[m_currentFrameSlot], nullptr);
    D3D12_RESOURCE_BARRIER rtBarrier = {};
    rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rtBarrier.Transition.pResource = m_rt[m_currentFrameSlot];
    rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_mainThreadDrawCmdList[1]->ResourceBarrier(1, &rtBarrier);
    m_mainThreadDrawCmdList[1]->Close();

    size_t bldTotal = 0;
    if (bldTab) {
        for (const BuilderList &bldList : *bldTab) {
            for (Builder *b : bldList) {
                if (b->commandList())
                    ++bldTotal;
            }
        }
    }
    const size_t cmdListCount = 2 + bldTotal;
    if (m_cmdListBatch.size() < cmdListCount)
        m_cmdListBatch.resize(cmdListCount);

    size_t batchPos = 0;
    m_cmdListBatch[batchPos++] = m_mainThreadDrawCmdList[0];
    if (bldTab) {
        for (const BuilderList &bldList : *bldTab) {
            for (Builder *b : bldList) {
                ID3D12CommandList *cmdList = b->commandList();
                if (cmdList)
                    m_cmdListBatch[batchPos++] = cmdList;
            }
        }
    }
    m_cmdListBatch[batchPos++] = m_mainThreadDrawCmdList[1];

    m_cmdQueue->ExecuteCommandLists(cmdListCount, m_cmdListBatch.data());

    HRESULT hr = m_swapchain->Present(PRESENT_SYNC_INTERVAL, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        handleLostDevice();
        return;
    } else if (FAILED(hr)) {
        logHr("Present failed", hr);
        return;
    }

    bumpFrameFence();
    m_currentFrameSlot = m_swapchain->GetCurrentBackBufferIndex();

    for (FrameExtraFunc f : m_postFrameFuncs)
        f();
}

void App::render()
{
    m_needsRender = false;
    if (m_zeroSize)
        return;

    //log("render (elapsed since last: %lld ms)", m_renderTimestamp.restart());

    if (!m_device) {
        if (!initialize()) {
            releaseResources();
            return;
        }
    }

    beginFrame();
    const BuilderTable *bldTab = nullptr;
    if (m_frameFunc) {
        bldTab = m_frameFunc();
        if (bldTab)
            postToBuildersAndWait(Builder::Event::Build, *bldTab);
    }
    endFrame(bldTab);
}

App *g_app = nullptr;

App::App(HINSTANCE hInstance, HWND hWnd, Builder::ThreadModel threadModel)
    : m_hInstance(hInstance),
      m_hWnd(hWnd),
      m_threadModel(threadModel)
{
    g_app = this;
}

App::~App()
{
    for (Builder *b : m_builders) {
        b->finish();
        delete b;
    }
    for (HANDLE event : m_waitEvents)
        CloseHandle(event);
    g_app = nullptr;
}

void App::addBuilder(Builder *b)
{
    m_builders.push_back(b);
    b->start();
}

void App::addBuilders(std::initializer_list<Builder *> args)
{
    for (Builder *b : args)
        addBuilder(b);
}

void App::deleteBuilder(Builder *b)
{
    auto it = std::find(m_builders.cbegin(), m_builders.cend(), b);
    if (it != m_builders.cend()) {
        Builder *b = *it;
        b->finish();
        m_builders.erase(it);
        delete b;
    }
}

void App::postToAllBuildersAndWait(Builder::Event e)
{
    postToBuildersAndWait(e, m_builders);
}

void App::postToBuildersAndWait(Builder::Event e, const BuilderTable &bldTab)
{
    for (const BuilderList &bldList : bldTab)
        postToBuildersAndWait(e, bldList);
}

void App::postToBuildersAndWait(Builder::Event e, const BuilderList &builders)
{
    if (m_threadModel == Builder::ThreadModel::Threaded) {
        const size_t builderCount = builders.size();
        const size_t batchSize = min(builderCount, MAXIMUM_WAIT_OBJECTS);
        const size_t existingSize = m_waitEvents.size();
        if (batchSize > existingSize) {
            m_waitEvents.resize(batchSize);
            for (size_t i = existingSize; i < batchSize; ++i)
                m_waitEvents[i] = CreateEvent(nullptr, false, false, nullptr);
        }
        for (size_t batchStart = 0; batchStart < builderCount; batchStart += batchSize) {
            for (size_t i = 0; i < batchSize; ++i)
                builders[batchStart + i]->postEvent(e, m_waitEvents[i]);
            WaitForMultipleObjects(batchSize, m_waitEvents.data(), true, INFINITE);
        }
    } else {
        for (Builder *b : builders)
            b->postEvent(e);
    }
}
