#ifndef BUILDER_H
#define BUILDER_H

#include "common.h"

struct Builder
{
    enum class ThreadModel {
        Threaded,
        NonThreaded
    };
    enum class Type {
        NoCommandList,
        GraphicsCommandList
    };
    enum class Event {
        Finish,
        Build,
        ReleaseResources
    };

    Builder(Type type);
    virtual ~Builder();

    Type type() const { return m_type; }
    ThreadModel threadModel() const { return m_threadModel; }
    bool isStarted() const { return m_started; }
    void postEvent(Event e, HANDLE waitEvent = nullptr);

    ID3D12CommandList *commandList() const;

protected:
    virtual void processEvent(Event e) = 0;

    Type m_type;
    ThreadModel m_threadModel;
    bool m_started = false;
    HANDLE m_msgEvent;
    std::mutex m_msgMutex;
    std::thread *m_thread = nullptr;
    using ThreadMessage = std::pair<Event, HANDLE>;
    std::vector<ThreadMessage> m_events;
    bool m_baseResReady = false;
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

struct ResourceBuilder : public Builder
{
    ResourceBuilder(Type type) : Builder(type) { }

    virtual void buildResources() = 0;
    virtual void releaseResources() = 0;

private:
    void processEvent(Event e) override;

    bool m_built = false;
};

#endif
