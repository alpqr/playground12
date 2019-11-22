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

ID3D12Resource *createBuffer(ID3D12Device *dev, Storage type, UINT64 size)
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

    ID3D12Resource *buf = nullptr;
    HRESULT hr = dev->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        initialState, nullptr, IID_PPV_ARGS(&buf));
    if (FAILED(hr)) {
        log("Failed to create buffer resoure", hr);
        return nullptr;
    }

    return buf;
}

}
