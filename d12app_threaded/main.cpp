#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <comdef.h>

#include <dxgi1_4.h>
#include <d3d12.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

const D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_0;
const int DEFAULT_WIDTH = 1280;
const int DEFAULT_HEIGHT = 720;
const int SWAPCHAIN_BUFFER_COUNT = 2;
const int FRAMES_IN_FLIGHT = SWAPCHAIN_BUFFER_COUNT;
const DXGI_FORMAT SWAPCHAIN_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
const bool ENABLE_DEBUG_LAYER = true;
const int ADAPTER_INDEX = -1;

static void log(const char *fmt, ...)
{
    char newLineFmt[512];
    strncpy_s(newLineFmt, sizeof(newLineFmt) - 1, fmt, _TRUNCATE);
    strcat_s(newLineFmt, sizeof(newLineFmt), "\n");

    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(msg, sizeof(msg), _TRUNCATE, newLineFmt, args);
    va_end(args);

    OutputDebugStringA(msg);
}

static void logHr(const char *msg, HRESULT hr)
{
    _com_error err(hr, nullptr);
#ifdef UNICODE
    log("%s: %ls", msg, err.ErrorMessage());
#else
    log("%s: %s", msg, err.ErrorMessage());
#endif
}

template<typename Int>
inline Int aligned(Int v, Int byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

struct DescHeapMgr
{
    void initialize(ID3D12Device *device);
    void releaseResources();

    D3D12_CPU_DESCRIPTOR_HANDLE allocate(D3D12_DESCRIPTOR_HEAP_TYPE type,
        UINT n,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    void release(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT n);
    UINT handleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

    static const UINT BUCKETS_PER_HEAP = 8;
    static const UINT DESCRIPTORS_PER_BUCKET = 32;
    static const UINT MAX_DESCRIPTORS_PER_HEAP = BUCKETS_PER_HEAP * DESCRIPTORS_PER_BUCKET;

    ID3D12Device *m_device = nullptr;
    std::mutex m_mutex;
    struct Heap {
        D3D12_DESCRIPTOR_HEAP_TYPE type;
        D3D12_DESCRIPTOR_HEAP_FLAGS flags;
        ID3D12DescriptorHeap *heap;
        D3D12_CPU_DESCRIPTOR_HANDLE start;
        UINT handleSize;
        UINT32 freeMap[BUCKETS_PER_HEAP];
    };
    std::vector<Heap> m_heaps;
    UINT m_handleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

D3D12_CPU_DESCRIPTOR_HANDLE DescHeapMgr::allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT n, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    if (n < 1)
        return {};
    std::lock_guard<std::mutex> lock(m_mutex);

    D3D12_CPU_DESCRIPTOR_HANDLE h = {};
    for (Heap &heap : m_heaps) {
        if (heap.type != type || heap.flags != flags)
            continue;

        for (UINT startBucket = 0; startBucket < _countof(heap.freeMap); ++startBucket) {
            UINT32 map = heap.freeMap[startBucket];
            while (map) {
                DWORD freePosInBucket;
                _BitScanForward(&freePosInBucket, map);
                const UINT freePos = freePosInBucket + startBucket * DESCRIPTORS_PER_BUCKET;

                // are there n consecutive free entries starting at freePos?
                bool canUse = true;
                for (UINT pos = freePos + 1; pos < freePos + n; ++pos) {
                    const UINT bucket = pos / DESCRIPTORS_PER_BUCKET;
                    const UINT indexInBucket = pos - bucket * DESCRIPTORS_PER_BUCKET;
                    if (!(heap.freeMap[bucket] & (1UL << indexInBucket))) {
                        canUse = false;
                        break;
                    }
                }

                if (canUse) {
                    for (UINT pos = freePos; pos < freePos + n; ++pos) {
                        const UINT bucket = pos / DESCRIPTORS_PER_BUCKET;
                        const UINT indexInBucket = pos - bucket * DESCRIPTORS_PER_BUCKET;
                        heap.freeMap[bucket] &= ~(1UL << indexInBucket);
                        //log("reserve descriptor handle, heap %p type %x bucket %u index %u",
                        //    &heap, type, bucket, indexInBucket);
                    }
                    h.ptr = heap.start.ptr + SIZE_T(freePos) * heap.handleSize;
                    return h;
                }

                map &= ~(1UL << freePosInBucket);
            }
        }
    }

    Heap heap;
    heap.type = type;
    heap.flags = flags;
    heap.handleSize = m_handleSizes[type];

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = MAX_DESCRIPTORS_PER_HEAP;
    heapDesc.Type = type;
    heapDesc.Flags = flags;

    HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_ID3D12DescriptorHeap, reinterpret_cast<void **>(&heap.heap));
    if (FAILED(hr)) {
        log("Failed to create descriptor heap", hr);
        return h;
    }

    heap.start = heap.heap->GetCPUDescriptorHandleForHeapStart();
    //log("new descriptor heap, type %x, start %llu", type, heap.start.ptr);

    for (int i = 0; i < _countof(heap.freeMap); ++i)
        heap.freeMap[i] = 0xFFFFFFFF;

    for (UINT pos = 0; pos < n; ++pos) {
        const UINT bucket = pos / DESCRIPTORS_PER_BUCKET;
        const UINT indexInBucket = pos - bucket * DESCRIPTORS_PER_BUCKET;
        heap.freeMap[bucket] &= ~(1UL << indexInBucket);
    }

    h = heap.start;
    m_heaps.push_back(heap);

    return h;
}

void DescHeapMgr::release(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT n)
{
    if (n < 1)
        return;
    std::lock_guard<std::mutex> lock(m_mutex);

    for (Heap &heap : m_heaps) {
        if(handle.ptr >= heap.start.ptr
            && handle.ptr + n * heap.handleSize <= heap.start.ptr + MAX_DESCRIPTORS_PER_HEAP * heap.handleSize)
        {
            const SIZE_T startPos = (handle.ptr - heap.start.ptr) / heap.handleSize;
            for (SIZE_T pos = startPos; pos < startPos + n; ++pos) {
                const UINT bucket = UINT(pos) / DESCRIPTORS_PER_BUCKET;
                const UINT indexInBucket = UINT(pos) - bucket * DESCRIPTORS_PER_BUCKET;
                heap.freeMap[bucket] |= 1UL << indexInBucket;
                //log("free descriptor handle, heap %p type %x bucket %u index %u",
                //    &heap, heap.type, bucket, indexInBucket);
            }
            return;
        }
    }
    log("Attempted to release untracked descriptor handle %llu", handle.ptr);
}

UINT DescHeapMgr::handleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
    return m_handleSizes[type];
}

void DescHeapMgr::initialize(ID3D12Device *device)
{
    m_device = device;
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        m_handleSizes[i] = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE(i));
}

void DescHeapMgr::releaseResources()
{
    for (Heap &heap : m_heaps)
        heap.heap->Release();
    m_heaps.clear();
    m_device = nullptr;
}

struct Timestamp
{
    Timestamp() { start(); }
    void start() { t = std::chrono::steady_clock::now(); }
    INT64 restart() { INT64 v = elapsed(); start(); return v; }
    INT64 elapsedNs() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t).count();
    }
    INT64 elapsed() const { return elapsedNs() / 1000000; }
    INT64 elapsedNsSince(const Timestamp &other) const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(t - other.t).count();
    }
    INT64 elapsedSince(const Timestamp &other) const { return elapsedNsSince(other) / 1000000; }
    std::chrono::steady_clock::time_point t;
};

struct App;

struct Builder
{
    enum Event {
        FinishEvent,
        BuildEvent,
        ReleaseResourcesEvent
    };

    Builder(App *app);
    virtual ~Builder();

    bool isStarted() const { return m_thread != nullptr; }
    void postEvent(Event e, HANDLE waitEvent = nullptr);

    ID3D12CommandList *commandList() const { return m_drawCmdList; }

protected:
    void start();
    void finish();
    void run();
    bool initializeBaseResources();
    void releaseBaseResources();

    virtual void processEvent(Event e) = 0;

    App *m_app;
    HANDLE m_msgEvent;
    std::mutex m_msgMutex;
    std::thread *m_thread = nullptr;
    using ThreadMessage = std::pair<Event, HANDLE>;
    std::vector<ThreadMessage> m_events;
    ID3D12CommandAllocator *m_cmdAllocator[FRAMES_IN_FLIGHT] = {};
    ID3D12GraphicsCommandList *m_drawCmdList = nullptr;

    friend struct App;
};

struct App
{
    App(HINSTANCE hInstance, HWND hWnd);
    ~App();

    bool initialize();
    void releaseResources();
    void waitGpu();
    void render();
    void resize(int newWidth, int newHeight);
    void logVidMemUsage();

    void bumpFrameFence();
    void waitForFrameFence();
    bool createSwapchainViews();
    void releaseSwapchainViews();
    void handleLostDevice();
    void beginFrame();
    void endFrame();

    DXGI_SAMPLE_DESC makeSampleDesc(DXGI_FORMAT format, int samples);
    ID3D12Resource *createDepthStencil(D3D12_CPU_DESCRIPTOR_HANDLE dsv, int width, int height, int samples);

    void requestUpdate() { m_needsRender = true; }
    void maybeUpdate() { if (m_needsRender) render(); }

    void addBuilder(Builder *b);
    void addBuilders(std::initializer_list<Builder *> args);
    void deleteBuilder(Builder *b);

    void postToAllBuildersAndWait(Builder::Event e);

    using FrameFunc = std::function<void()>;
    void addPreFrameFunc(FrameFunc f) { m_preFrameFuncs.push_back(f); }
    void addPostFrameFunc(FrameFunc f) { m_postFrameFuncs.push_back(f); }

    HINSTANCE m_hInstance;
    HWND m_hWnd = 0;
    int m_width = DEFAULT_WIDTH;
    int m_height = DEFAULT_HEIGHT;
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

Builder::Builder(App *app)
    : m_app(app)
{
    m_msgEvent = CreateEvent(nullptr, false, false, nullptr);
}

Builder::~Builder()
{
    CloseHandle(m_msgEvent);
}

void Builder::start()
{
    if (isStarted())
        return;

    m_thread = new std::thread(std::bind(&Builder::run, this));
}

void Builder::finish()
{
    if (!isStarted())
        return;

    postEvent(FinishEvent);
    m_thread->join();
    delete m_thread;
    m_thread = nullptr;
}

void Builder::postEvent(Event e, HANDLE waitEvent)
{
    {
        std::lock_guard<std::mutex> lock(m_msgMutex);
        m_events.push_back(std::make_pair(e, waitEvent));
    }
    SetEvent(m_msgEvent);
}

bool Builder::initializeBaseResources()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        HRESULT hr = m_app->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_ID3D12CommandAllocator,
            reinterpret_cast<void **>(&m_cmdAllocator[i]));
        if (FAILED(hr)) {
            logHr("Failed to create command allocator", hr);
            return false;
        }
    }

    HRESULT hr = m_app->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_cmdAllocator[0], nullptr, IID_ID3D12GraphicsCommandList,
        reinterpret_cast<void **>(&m_drawCmdList));
    if (FAILED(hr)) {
        log("Failed to create graphics command list", hr);
        return false;
    }
    m_drawCmdList->Close();

    return true;
}

void Builder::releaseBaseResources()
{
    if (m_drawCmdList) {
        m_drawCmdList->Release();
        m_drawCmdList = nullptr;
    }

    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        if (m_cmdAllocator[i]) {
            m_cmdAllocator[i]->Release();
            m_cmdAllocator[i] = nullptr;
        }
    }
}

void Builder::run()
{
    for (; ;) {
        WaitForSingleObject(m_msgEvent, INFINITE);

        m_msgMutex.lock();
        std::vector<ThreadMessage> events = std::move(m_events);
        m_msgMutex.unlock();

        for (const ThreadMessage &e : events) {
            if (e.first == FinishEvent)
                return;
            if (e.first == BuildEvent) {
                if (!m_drawCmdList) {
                    if (!initializeBaseResources()) {
                        releaseBaseResources();
                        continue;
                    }
                }
                m_cmdAllocator[m_app->m_currentFrameSlot]->Reset();
                m_drawCmdList->Reset(m_cmdAllocator[m_app->m_currentFrameSlot], nullptr);
            }
            processEvent(e.first);
            if (e.first == ReleaseResourcesEvent)
                releaseBaseResources();
            else if (e.first == BuildEvent)
                m_drawCmdList->Close();
            if (e.second)
                SetEvent(e.second);
        }
    }
}

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

DXGI_SAMPLE_DESC App::makeSampleDesc(DXGI_FORMAT format, int samples)
{
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    if (samples > 1) {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaInfo = {};
        msaaInfo.Format = format;
        msaaInfo.SampleCount = samples;
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaInfo, sizeof(msaaInfo)))) {
            if (msaaInfo.NumQualityLevels > 0) {
                sampleDesc.Count = samples;
                sampleDesc.Quality = msaaInfo.NumQualityLevels - 1;
            } else {
                log("No quality levels for multisampling with sample count %d", samples);
            }
        } else {
            log("Failed to query multisample quality levels for sample count %d", samples);
        }
    }

    return sampleDesc;
}

ID3D12Resource *App::createDepthStencil(D3D12_CPU_DESCRIPTOR_HANDLE dsv, int width, int height, int samples)
{
    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC bufDesc = {};
    bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    bufDesc.Width = width;
    bufDesc.Height = height;
    bufDesc.DepthOrArraySize = 1;
    bufDesc.MipLevels = 1;
    bufDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    bufDesc.SampleDesc = makeSampleDesc(bufDesc.Format, samples);
    bufDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    bufDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    ID3D12Resource *resource = nullptr;
    if (FAILED(m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &bufDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue,
        IID_ID3D12Resource, reinterpret_cast<void **>(&resource))))
    {
        log("Failed to create depth-stencil buffer of size %dx%d", width, height);
        return nullptr;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.ViewDimension = bufDesc.SampleDesc.Count <= 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;

    m_device->CreateDepthStencilView(resource, &depthStencilDesc, dsv);

    return resource;
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
    m_ds = createDepthStencil(m_dsv, m_width, m_height, 1);
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
            log("Failed to create graphics command list", hr);
            return false;
        }
        m_mainThreadDrawCmdList[i]->Close();
    }

    return true;
}

void App::releaseResources()
{
    waitGpu();

    postToAllBuildersAndWait(Builder::ReleaseResourcesEvent);

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

void App::resize(int newWidth, int newHeight)
{
    if (m_width == newWidth && m_height == newHeight)
        return;

    m_width = newWidth;
    m_height = newHeight;
    m_zeroSize = m_width <= 0 || m_height <= 0; // f.ex. when minimized
    if (m_zeroSize)
        m_width = m_height = 1;

    log("resize %d %d", m_width, m_height);

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

    for (FrameFunc f : m_preFrameFuncs)
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

void App::endFrame()
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

    const size_t builderCount = m_builders.size();
    m_cmdListBatch.resize(2 + builderCount);
    m_cmdListBatch[0] = m_mainThreadDrawCmdList[0];
    for (size_t i = 0; i < builderCount; ++i)
        m_cmdListBatch[i + 1] = m_builders[i]->commandList();
    m_cmdListBatch[1 + builderCount] = m_mainThreadDrawCmdList[1];
    m_cmdQueue->ExecuteCommandLists(m_cmdListBatch.size(), m_cmdListBatch.data());

    HRESULT hr = m_swapchain->Present(1, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        handleLostDevice();
        return;
    } else if (FAILED(hr)) {
        logHr("Present failed", hr);
        return;
    }

    bumpFrameFence();
    m_currentFrameSlot = m_swapchain->GetCurrentBackBufferIndex();

    for (FrameFunc f : m_postFrameFuncs)
        f();
}

void App::render()
{
    m_needsRender = false;
    if (m_zeroSize)
        return;

    log("render (elapsed since last: %lld ms)", m_renderTimestamp.restart());

    if (!m_device) {
        if (!initialize()) {
            releaseResources();
            return;
        }
    }

    beginFrame();

    postToAllBuildersAndWait(Builder::BuildEvent);

    endFrame();
}

static App *g_app;

App::App(HINSTANCE hInstance, HWND hWnd)
    : m_hInstance(hInstance),
      m_hWnd(hWnd)
{
    g_app = this;
}

App::~App()
{
    for (Builder *b : m_builders) {
        b->finish();
        delete b;
    }
    m_builders.clear();
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
    const size_t count = m_builders.size();
    const size_t eventsToResetCount = m_waitEvents.size();
    m_waitEvents.resize(count);
    for (size_t i = 0; i < eventsToResetCount; ++i)
        ResetEvent(m_waitEvents[i]);
    for (size_t i = eventsToResetCount; i < count; ++i)
        m_waitEvents[i] = CreateEvent(nullptr, false, false, nullptr);
    for (size_t i = 0; i < count; ++i)
        m_builders[i]->postEvent(e, m_waitEvents[i]);
    WaitForMultipleObjects(count, m_waitEvents.data(), true, INFINITE);
}

struct BldDefaultRtInit : public Builder
{
    BldDefaultRtInit(App *app) : Builder(app) { }
    void processEvent(Event e) override;
};

void BldDefaultRtInit::processEvent(Event e)
{
    if (e == BuildEvent) {
        D3D12_CPU_DESCRIPTOR_HANDLE *rtv = &m_app->m_rtv[m_app->m_currentFrameSlot];
        m_drawCmdList->OMSetRenderTargets(1, rtv, false, &m_app->m_dsv);
        const float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        m_drawCmdList->ClearRenderTargetView(*rtv, clearColor, 0, nullptr);
        m_drawCmdList->ClearDepthStencilView(m_app->m_dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    }
}

static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        if (g_app)
            g_app->releaseResources();
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        if (g_app)
            g_app->render();
    }
        return 0;

    case WM_SIZE:
    {
        const int newWidth = int(lParam & 0xFFFF);
        const int newHeight = int(lParam >> 16);
        if (g_app)
            g_app->resize(newWidth, newHeight);
    }
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

_Use_decl_annotations_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = _T("d12app_threaded");
    if (!RegisterClassEx(&windowClass)) {
        OutputDebugString(_T("RegisterClassEx failed"));
        return EXIT_FAILURE;
    }

    const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    RECT windowRect = { 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT };
    AdjustWindowRect(&windowRect, windowStyle, false);

    HWND window = CreateWindow(windowClass.lpszClassName, _T("d12app_threaded"), windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!window) {
        OutputDebugString(_T("CreateWindow failed"));
        return EXIT_FAILURE;
    }

    ShowWindow(window, nCmdShow);

    App app(hInstance, window);

    app.addBuilders({
        new BldDefaultRtInit(&app)
    });

    app.addPostFrameFunc([&app] { app.requestUpdate(); });

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            app.maybeUpdate();
        }
    }

    return EXIT_SUCCESS;
}
