#ifndef RENDER_H
#define RENDER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>

const bool ENABLE_DEBUG_LAYER = true;

const uint32_t DEFAULT_WIDTH = 1280;
const uint32_t DEFAULT_HEIGHT = 720;

struct Device
{
    IDXGIFactory2 *dxgiFactory;
    ID3D11Device *device;
    ID3D11DeviceContext1 *context;
    D3D_FEATURE_LEVEL featureLevel;
};

struct Swapchain
{
    IDXGISwapChain1 *swapchain;
    ID3D11Texture2D *tex;
    ID3D11RenderTargetView *rtv;
    ID3D11Texture2D *ds;
    ID3D11DepthStencilView *dsv;
    uint32_t width;
    uint32_t height;
};

bool createDevice(Device *dev);
void destroyDevice(Device *dev);
bool createSwapchain(Device *dev, HWND hwnd, uint32_t width, uint32_t height, Swapchain *swapchain);
void createSwapchainBuffers(Device *dev, Swapchain *swapchain);
void destroySwapchain(Swapchain *swapchain);
void resizeSwapchain(Device *dev, Swapchain *swapchain, uint32_t width, uint32_t height);

void log(const char *fmt, ...);

#endif
