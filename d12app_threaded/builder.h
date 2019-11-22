#ifndef BUILDER_H
#define BUILDER_H

#include "common.h"

struct Builder
{
    enum class Event {
        Finish,
        Build,
        ReleaseResources
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

#endif
