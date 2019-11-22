#include "draw.h"
#include "app.h"

void registerDrawCmdListBuilders()
{
    g_app->addBuilders({
        new BldDefaultRtInit
    });
}

void BldDefaultRtInit::processEvent(Event e)
{
    if (e == Event::Build) {
        D3D12_CPU_DESCRIPTOR_HANDLE *rtv = &m_app->m_rtv[m_app->m_currentFrameSlot];
        m_drawCmdList->OMSetRenderTargets(1, rtv, false, &m_app->m_dsv);
        const float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        m_drawCmdList->ClearRenderTargetView(*rtv, clearColor, 0, nullptr);
        m_drawCmdList->ClearDepthStencilView(m_app->m_dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    }
}
