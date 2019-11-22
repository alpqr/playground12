#ifndef BUILDER_H
#define BUILDER_H

#include "common.h"

struct Builder
{
    enum class Type {
        Threaded,
        NonThreaded
    };
    enum class Event {
        Finish,
        Build,
        ReleaseResources
    };

    Builder();
    virtual ~Builder();

    Type type() const { return m_type; }
    bool isStarted() const { return m_started; }
    void postEvent(Event e, HANDLE waitEvent = nullptr);

    ID3D12CommandList *commandList() const { return m_drawCmdList; }

protected:
    virtual void processEvent(Event e) = 0;

    Type m_type;
    bool m_started = false;
    HANDLE m_msgEvent;
    std::mutex m_msgMutex;
    std::thread *m_thread = nullptr;
    using ThreadMessage = std::pair<Event, HANDLE>;
    std::vector<ThreadMessage> m_events;
    ID3D12CommandAllocator *m_cmdAllocator[FRAMES_IN_FLIGHT] = {};
    ID3D12GraphicsCommandList *m_drawCmdList = nullptr;

private:
    void start();
    void finish();
    void run();
    void invokeProcessEvent(const ThreadMessage &e);
    bool initializeBaseResources();
    void releaseBaseResources();
    friend struct App;
};

#endif
