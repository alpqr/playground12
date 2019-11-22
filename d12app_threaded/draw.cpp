#include "draw.h"
#include "app.h"

BuilderHost::BuilderHost()
{
    defaultRtInit = new BldDefaultRtInit;

    g_app->addBuilders({
        defaultRtInit
    });

    bldTab = { { defaultRtInit } };
}

const BuilderTable *BuilderHost::frameFunc()
{
    return &bldTab;
}

void BldDefaultRtInit::processEvent(Event e)
{
    if (e == Event::Build) {
        D3D12_CPU_DESCRIPTOR_HANDLE *rtv = &g_app->m_rtv[g_app->m_currentFrameSlot];
        m_drawCmdList->OMSetRenderTargets(1, rtv, false, &g_app->m_dsv);
        const float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        m_drawCmdList->ClearRenderTargetView(*rtv, clearColor, 0, nullptr);
        m_drawCmdList->ClearDepthStencilView(g_app->m_dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    }
}
