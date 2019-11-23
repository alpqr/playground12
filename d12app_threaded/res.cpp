#include "res.h"

namespace Res {

DXGI_SAMPLE_DESC makeSampleDesc(ID3D12Device *dev, DXGI_FORMAT format, UINT samples)
{
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    if (samples > 1) {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaInfo = {};
        msaaInfo.Format = format;
        msaaInfo.SampleCount = samples;
        if (SUCCEEDED(dev->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaInfo, sizeof(msaaInfo)))) {
            if (msaaInfo.NumQualityLevels > 0) {
                sampleDesc.Count = samples;
                sampleDesc.Quality = msaaInfo.NumQualityLevels - 1;
            } else {
                log("No quality levels for multisampling with sample count %u", samples);
            }
        } else {
            log("Failed to query multisample quality levels for sample count %u", samples);
        }
    }

    return sampleDesc;
}

ID3D12Resource *createDepthStencil(ID3D12Device *dev, D3D12_CPU_DESCRIPTOR_HANDLE dsv, UINT width, UINT height, UINT samples)
{
    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = max(1U, width);
    desc.Height = max(1U, height);
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc = makeSampleDesc(dev, desc.Format, samples);
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    ID3D12Resource *resource = nullptr;
    if (FAILED(dev->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue,
        IID_ID3D12Resource, reinterpret_cast<void **>(&resource))))
    {
        log("Failed to create depth-stencil buffer of size %ux%u", width, height);
        return nullptr;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.ViewDimension = desc.SampleDesc.Count <= 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;

    dev->CreateDepthStencilView(resource, &depthStencilDesc, dsv);

    return resource;
}

ID3D12Resource *createBuffer(ID3D12Device *dev, Storage type, UINT64 size, D3D12_RESOURCE_FLAGS resourceFlags)
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    D3D12_RESOURCE_STATES initialState = {};
    switch (type) {
    case Storage::Device:
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        initialState = D3D12_RESOURCE_STATE_COMMON;
        break;
    case Storage::HostToDevice:
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
        break;
    case Storage::DeviceToHost:
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        break;
    }

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = max(1LLU, size);
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = resourceFlags;

    ID3D12Resource *buf = nullptr;
    HRESULT hr = dev->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        initialState, nullptr, IID_PPV_ARGS(&buf));
    if (FAILED(hr)) {
        log("Failed to create buffer resoure", hr);
        return nullptr;
    }

    return buf;
}

void transitionResource(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
}

ID3D12RootSignature *createRootSignature(ID3D12Device *dev,
    UINT paramCount, const D3D12_ROOT_PARAMETER *params,
    UINT staticSamplerCount, const D3D12_STATIC_SAMPLER_DESC *staticSamplers)
{
    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = paramCount;
    desc.pParameters = params;
    desc.NumStaticSamplers = staticSamplerCount;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob *sig = nullptr;
    ID3DBlob *err = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &err);
    if (FAILED(hr)) {
        logHr("Failed to serialize root signature", hr);
        if (err) {
            log("%s", static_cast<char *>(err->GetBufferPointer()));
            err->Release();
        }
        return nullptr;
    }

    ID3D12RootSignature *rootSig;
    hr = dev->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_ID3D12RootSignature, reinterpret_cast<void **>(&rootSig));
    sig->Release();
    if (FAILED(hr)) {
        logHr("Failed to create root signature", hr);
        return nullptr;
    }

    return rootSig;
}

ID3D12PipelineState *createSimplePso(ID3D12Device *dev,
    ID3D12RootSignature *rootSig,
    const void *vs, SIZE_T vsSize,
    const void *ps, SIZE_T psSize,
    const D3D12_INPUT_ELEMENT_DESC *inputElements, UINT inputElementCount)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = rootSig;

    psoDesc.VS.pShaderBytecode = vs;
    psoDesc.VS.BytecodeLength = vsSize;

    psoDesc.PS.pShaderBytecode = ps;
    psoDesc.PS.BytecodeLength = psSize;

    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.BlendState = blendDesc;

    psoDesc.SampleMask = UINT_MAX;

    D3D12_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rastDesc.CullMode = D3D12_CULL_MODE_NONE;
    rastDesc.FrontCounterClockwise = true;
    rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rastDesc.DepthClipEnable = true;
    psoDesc.RasterizerState = rastDesc;

    psoDesc.DepthStencilState.DepthEnable = true;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

    psoDesc.InputLayout.pInputElementDescs = inputElements;
    psoDesc.InputLayout.NumElements = inputElementCount;

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = SWAPCHAIN_FORMAT;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    ID3D12PipelineState *pso;
    HRESULT hr = dev->CreateGraphicsPipelineState(&psoDesc, IID_ID3D12PipelineState, reinterpret_cast<void **>(&pso));
    if (FAILED(hr)) {
        log("Failed to create graphics pipeline state", hr);
        return nullptr;
    }

    return pso;
}

} // namespace
