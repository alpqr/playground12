#ifndef RES_H
#define RES_H

#include "common.h"

namespace Res {

DXGI_SAMPLE_DESC makeSampleDesc(ID3D12Device *dev, DXGI_FORMAT format, UINT samples);
ID3D12Resource *createDepthStencil(ID3D12Device *dev, D3D12_CPU_DESCRIPTOR_HANDLE dsv, UINT width, UINT height, UINT samples);

enum class Storage {
    Device,
    HostToDevice,
    DeviceToHost
};
ID3D12Resource *createBuffer(ID3D12Device *dev, Storage type, UINT64 size, D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE);

void transitionResource(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

ID3D12RootSignature *createRootSignature(ID3D12Device *dev,
    UINT paramCount, const D3D12_ROOT_PARAMETER *params,
    UINT staticSamplerCount = 0, const D3D12_STATIC_SAMPLER_DESC *staticSamplers = nullptr);

ID3D12PipelineState *createSimplePso(ID3D12Device *dev,
    ID3D12RootSignature *rootSig,
    const void *vs, SIZE_T vsSize,
    const void *ps, SIZE_T psSize,
    const D3D12_INPUT_ELEMENT_DESC *inputElements, UINT inputElementCount);

} // namespace

#endif
