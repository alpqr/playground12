#ifndef DESCHEAPMGR_H
#define DESCHEAPMGR_H

#include "common.h"

struct DescHeapMgr
{
    void initialize(ID3D12Device *device);
    void releaseResources();

    D3D12_CPU_DESCRIPTOR_HANDLE allocate(D3D12_DESCRIPTOR_HEAP_TYPE type,
        UINT n,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    void release(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT n);
    UINT handleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

    static const UINT BUCKETS_PER_HEAP = 8;
    static const UINT DESCRIPTORS_PER_BUCKET = 32;
    static const UINT MAX_DESCRIPTORS_PER_HEAP = BUCKETS_PER_HEAP * DESCRIPTORS_PER_BUCKET;

    ID3D12Device *m_device = nullptr;
    std::mutex m_mutex;
    struct Heap {
        D3D12_DESCRIPTOR_HEAP_TYPE type;
        D3D12_DESCRIPTOR_HEAP_FLAGS flags;
        ID3D12DescriptorHeap *heap;
        D3D12_CPU_DESCRIPTOR_HANDLE start;
        UINT handleSize;
        UINT32 freeMap[BUCKETS_PER_HEAP];
    };
    std::vector<Heap> m_heaps;
    UINT m_handleSizes[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

#endif
