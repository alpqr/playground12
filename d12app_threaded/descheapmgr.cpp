#include "descheapmgr.h"

D3D12_CPU_DESCRIPTOR_HANDLE DescHeapMgr::allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT n, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    if (n < 1)
        return {};
    std::lock_guard<std::mutex> lock(m_mutex);

    D3D12_CPU_DESCRIPTOR_HANDLE h = {};
    for (Heap &heap : m_heaps) {
        if (heap.type != type || heap.flags != flags)
            continue;

        for (UINT startBucket = 0; startBucket < _countof(heap.freeMap); ++startBucket) {
            UINT32 map = heap.freeMap[startBucket];
            while (map) {
                DWORD freePosInBucket;
                _BitScanForward(&freePosInBucket, map);
                const UINT freePos = freePosInBucket + startBucket * DESCRIPTORS_PER_BUCKET;

                // are there n consecutive free entries starting at freePos?
                bool canUse = true;
                for (UINT pos = freePos + 1; pos < freePos + n; ++pos) {
                    const UINT bucket = pos / DESCRIPTORS_PER_BUCKET;
                    const UINT indexInBucket = pos - bucket * DESCRIPTORS_PER_BUCKET;
                    if (!(heap.freeMap[bucket] & (1UL << indexInBucket))) {
                        canUse = false;
                        break;
                    }
                }

                if (canUse) {
                    for (UINT pos = freePos; pos < freePos + n; ++pos) {
                        const UINT bucket = pos / DESCRIPTORS_PER_BUCKET;
                        const UINT indexInBucket = pos - bucket * DESCRIPTORS_PER_BUCKET;
                        heap.freeMap[bucket] &= ~(1UL << indexInBucket);
                        //log("reserve descriptor handle, heap %p type %x bucket %u index %u",
                        //    &heap, type, bucket, indexInBucket);
                    }
                    h.ptr = heap.start.ptr + SIZE_T(freePos) * heap.handleSize;
                    return h;
                }

                map &= ~(1UL << freePosInBucket);
            }
        }
    }

    Heap heap;
    heap.type = type;
    heap.flags = flags;
    heap.handleSize = m_handleSizes[type];

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = MAX_DESCRIPTORS_PER_HEAP;
    heapDesc.Type = type;
    heapDesc.Flags = flags;

    HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_ID3D12DescriptorHeap, reinterpret_cast<void **>(&heap.heap));
    if (FAILED(hr)) {
        log("Failed to create descriptor heap", hr);
        return h;
    }

    heap.start = heap.heap->GetCPUDescriptorHandleForHeapStart();
    //log("new descriptor heap, type %x, start %llu", type, heap.start.ptr);

    for (int i = 0; i < _countof(heap.freeMap); ++i)
        heap.freeMap[i] = 0xFFFFFFFF;

    for (UINT pos = 0; pos < n; ++pos) {
        const UINT bucket = pos / DESCRIPTORS_PER_BUCKET;
        const UINT indexInBucket = pos - bucket * DESCRIPTORS_PER_BUCKET;
        heap.freeMap[bucket] &= ~(1UL << indexInBucket);
    }

    h = heap.start;
    m_heaps.push_back(heap);

    return h;
}

void DescHeapMgr::release(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT n)
{
    if (n < 1)
        return;
    std::lock_guard<std::mutex> lock(m_mutex);

    for (Heap &heap : m_heaps) {
        if(handle.ptr >= heap.start.ptr
            && handle.ptr + n * heap.handleSize <= heap.start.ptr + MAX_DESCRIPTORS_PER_HEAP * heap.handleSize)
        {
            const SIZE_T startPos = (handle.ptr - heap.start.ptr) / heap.handleSize;
            for (SIZE_T pos = startPos; pos < startPos + n; ++pos) {
                const UINT bucket = UINT(pos) / DESCRIPTORS_PER_BUCKET;
                const UINT indexInBucket = UINT(pos) - bucket * DESCRIPTORS_PER_BUCKET;
                heap.freeMap[bucket] |= 1UL << indexInBucket;
                //log("free descriptor handle, heap %p type %x bucket %u index %u",
                //    &heap, heap.type, bucket, indexInBucket);
            }
            return;
        }
    }
    log("Attempted to release untracked descriptor handle %llu", handle.ptr);
}

UINT DescHeapMgr::handleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
    return m_handleSizes[type];
}

void DescHeapMgr::initialize(ID3D12Device *device)
{
    m_device = device;
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        m_handleSizes[i] = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE(i));
}

void DescHeapMgr::releaseResources()
{
    for (Heap &heap : m_heaps)
        heap.heap->Release();
    m_heaps.clear();
    m_device = nullptr;
}
