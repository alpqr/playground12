#ifndef COMMON_H
#define COMMON_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <comdef.h>

#include <dxgi1_4.h>
#include <d3d12.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>

void log(const char *fmt, ...);
void logHr(const char *msg, HRESULT hr);

template<typename Int>
inline Int aligned(Int v, Int byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

const D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_0;
const UINT DEFAULT_WIDTH = 1280;
const UINT DEFAULT_HEIGHT = 720;
const int SWAPCHAIN_BUFFER_COUNT = 2;
const int FRAMES_IN_FLIGHT = SWAPCHAIN_BUFFER_COUNT;
const DXGI_FORMAT SWAPCHAIN_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
const bool ENABLE_DEBUG_LAYER = true;
const int ADAPTER_INDEX = -1;
const UINT PRESENT_SYNC_INTERVAL = 1;

struct App;
extern App *g_app;

#endif
