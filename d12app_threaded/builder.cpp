#include "builder.h"
#include "app.h"

Builder::Builder()
    : m_type(g_app->builderType())
{
    if (m_type == Type::Threaded)
        m_msgEvent = CreateEvent(nullptr, false, false, nullptr);
}

Builder::~Builder()
{
    if (m_type == Type::Threaded)
        CloseHandle(m_msgEvent);
}

void Builder::start()
{
    if (isStarted())
        return;
    if (m_type == Type::Threaded)
        m_thread = new std::thread(std::bind(&Builder::run, this));
    m_started = true;
}

void Builder::finish()
{
    if (!isStarted())
        return;
    postEvent(Event::Finish);
    if (m_type == Type::Threaded) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    m_started = false;
}

void Builder::postEvent(Event e, HANDLE waitEvent)
{
    if (m_type == Type::Threaded) {
        {
            std::lock_guard<std::mutex> lock(m_msgMutex);
            m_events.push_back(std::make_pair(e, waitEvent));
        }
        SetEvent(m_msgEvent);
    } else {
        const ThreadMessage msg = std::make_pair(e, nullptr);
        invokeProcessEvent(msg);
    }
}

bool Builder::initializeBaseResources()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        HRESULT hr = g_app->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_ID3D12CommandAllocator,
            reinterpret_cast<void **>(&m_cmdAllocator[i]));
        if (FAILED(hr)) {
            logHr("Failed to create command allocator", hr);
            return false;
        }
    }

    HRESULT hr = g_app->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
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
    assert(m_type == Type::Threaded);
    for (; ;) {
        WaitForSingleObject(m_msgEvent, INFINITE);

        m_msgMutex.lock();
        std::vector<ThreadMessage> events = std::move(m_events);
        m_msgMutex.unlock();

        for (const ThreadMessage &e : events) {
            if (e.first == Event::Finish)
                return;
            invokeProcessEvent(e);
            if (e.second)
                SetEvent(e.second);
        }
    }
}

void Builder::invokeProcessEvent(const ThreadMessage &e)
{
    if (e.first == Event::Build) {
        if (!m_drawCmdList) {
            if (!initializeBaseResources()) {
                releaseBaseResources();
                return;
            }
        }
        m_cmdAllocator[g_app->m_currentFrameSlot]->Reset();
        m_drawCmdList->Reset(m_cmdAllocator[g_app->m_currentFrameSlot], nullptr);
    }
    processEvent(e.first);
    if (e.first == Event::ReleaseResources)
        releaseBaseResources();
    else if (e.first == Event::Build)
        m_drawCmdList->Close();
}
