#include "draw.h"
#include "app.h"
#include "res.h"

BuilderHost::BuilderHost()
{
    m_builders.res0 = new BldRes0;
    m_builders.res1 = new BldRes1;
    m_builders.defaultRt = new BldDefaultRt;

    g_app->addBuilders({
        m_builders.res0,
        m_builders.res1,
        m_builders.defaultRt
    });

    m_res = { m_builders.res0, m_builders.res1 };

    m_bldTab = {
        { },
        { m_builders.defaultRt }
    };
}

const BuilderTable *BuilderHost::frame()
{
    if (!m_resourcesBuilt) {
        m_bldTab[0] = m_res;
        m_resourcesBuilt = true;
    } else if (m_bldTab[0].size()) {
        m_bldTab[0].clear();
    }

    return &m_bldTab;
}

void BuilderHost::releaseResourcesNotify()
{
    m_resourcesBuilt = false;
}

struct Material
{
    ID3D12RootSignature *rootSig = nullptr;
    ID3D12PipelineState *pso = nullptr;

    void release() {
        if (pso) {
            pso->Release();
            pso = nullptr;
        }
        if (rootSig) {
            rootSig->Release();
            rootSig = nullptr;
        }
    }
};

struct
{
    Material flatColorMaterial;
    ID3D12Resource *vbuf = nullptr;
} d;

void BldRes0::buildResources()
{
    log("build graphics resources 0");

    {
        D3D12_ROOT_PARAMETER param;
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param.Descriptor.ShaderRegister = 0; // b0
        param.Descriptor.RegisterSpace = 0;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        d.flatColorMaterial.rootSig = Res::createRootSignature(g_app->m_device, 1, &param);

        D3D12_INPUT_ELEMENT_DESC inputElem;
        inputElem.SemanticName = "POSITION";
        inputElem.SemanticIndex = 0;
        inputElem.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputElem.InputSlot = 0;
        inputElem.AlignedByteOffset = 0;
        inputElem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        inputElem.InstanceDataStepRate = 0;

        const char *vs = 0;
        SIZE_T vsSize = 0;
        const char *ps = 0;
        SIZE_T psSize = 0;

/*        d.flatColorMaterial.pso = Res::createSimplePso(g_app->m_device, d.flatColorMaterial.rootSig,
            vs, vsSize, ps, psSize,
            &inputElem, 1);*/
    }
}

void BldRes0::releaseResources()
{
    log("release graphics resources 0");

    d.flatColorMaterial.release();
}

void BldRes1::buildResources()
{
    log("build graphics resources 1");

    d.vbuf = Res::createBuffer(g_app->m_device, Res::Storage::Device, 64);
}

void BldRes1::releaseResources()
{
    log("release graphics resources 1");

    if (d.vbuf) {
        d.vbuf->Release();
        d.vbuf = nullptr;
    }
}

void BldDefaultRt::processEvent(Event e)
{
    if (e == Event::Build) {
        D3D12_CPU_DESCRIPTOR_HANDLE *rtv = &g_app->m_rtv[g_app->m_currentFrameSlot];
        m_drawCmdList->OMSetRenderTargets(1, rtv, false, &g_app->m_dsv);
        const float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        m_drawCmdList->ClearRenderTargetView(*rtv, clearColor, 0, nullptr);
        m_drawCmdList->ClearDepthStencilView(g_app->m_dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    }
}
