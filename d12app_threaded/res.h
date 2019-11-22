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
    ID3D12Resource *createBuffer(ID3D12Device *dev, Storage type, UINT64 size);
}

#endif
