#include "render.h"

#define RELEASE(obj) { if (obj) { obj->Release(); obj = nullptr; } }

bool createDevice(Device *dev)
{
    HRESULT hr = CreateDXGIFactory2(0, IID_IDXGIFactory2, reinterpret_cast<void **>(&dev->dxgiFactory));
    if (FAILED(hr)) {
        log("CreateDXGIFactory2() failed to create DXGI factory: 0x%X", hr);
        return false;
    }

    ID3D11DeviceContext *ctx = nullptr;
    UINT flags = 0;
    if (ENABLE_DEBUG_LAYER)
        flags |= D3D11_CREATE_DEVICE_DEBUG;

    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                           nullptr, 0, D3D11_SDK_VERSION,
                           &dev->device, &dev->featureLevel, &ctx);
    if (FAILED(hr)) {
        log("Failed to create D3D11 device and context: 0x%X", hr);
        return false;
    }

    if (SUCCEEDED(ctx->QueryInterface(IID_ID3D11DeviceContext1, reinterpret_cast<void **>(&dev->context)))) {
        ctx->Release();
    } else {
        log("ID3D11DeviceContext1 not supported");
        return false;
    }

    return true;
}

void destroyDevice(Device *dev)
{
    RELEASE(dev->context);
    RELEASE(dev->device);
    RELEASE(dev->dxgiFactory);
}

bool createSwapchain(Device *dev, HWND hwnd, uint32_t width, uint32_t height, Swapchain *swapchain)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    IDXGISwapChain1 *sc = nullptr;
    HRESULT hr = dev->dxgiFactory->CreateSwapChainForHwnd(dev->device, hwnd, &desc, nullptr, nullptr, &sc);
    if (FAILED(hr)) {
        log("Failed to create D3D11 swapchain: 0x%X", hr);
        return false;
    }

    swapchain->swapchain = sc;
    swapchain->width = width;
    swapchain->height = height;

    createSwapchainBuffers(dev, swapchain);

    return true;
}

void destroySwapchain(Swapchain *swapchain)
{
    RELEASE(swapchain->dsv);
    RELEASE(swapchain->ds);
    RELEASE(swapchain->rtv);
    RELEASE(swapchain->tex);
    RELEASE(swapchain->swapchain);
}

void createSwapchainBuffers(Device *dev, Swapchain *swapchain)
{
    ID3D11Texture2D *tex = nullptr;
    HRESULT hr = swapchain->swapchain->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void **>(&tex));
    if (FAILED(hr)) {
        log("Failed to query swapchain backbuffer: 0x%X", hr);
        return;
    }

    ID3D11RenderTargetView *rtv = nullptr;
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = dev->device->CreateRenderTargetView(tex, &rtvDesc, &rtv);
    if (FAILED(hr)) {
        log("Failed to create rtv for swapchain backbuffer: 0x%X", hr);
        tex->Release();
        return;
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = swapchain->width;
    texDesc.Height = swapchain->height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D *ds = nullptr;
    hr = dev->device->CreateTexture2D(&texDesc, nullptr, &ds);
    if (FAILED(hr)) {
        log("Failed to create depth-stencil buffer: 0x%X", hr);
        tex->Release();
        rtv->Release();
        return;
    }

    ID3D11DepthStencilView *dsv = nullptr;
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = dev->device->CreateDepthStencilView(ds, &dsvDesc, &dsv);
    if (FAILED(hr)) {
        log("Failed to create dsv: 0x%X", hr);
        tex->Release();
        rtv->Release();
        ds->Release();
        return;
    }

    swapchain->tex = tex;
    swapchain->rtv = rtv;
    swapchain->ds = ds;
    swapchain->dsv = dsv;
}

void resizeSwapchain(Device *dev, Swapchain *swapchain, uint32_t width, uint32_t height)
{
    RELEASE(swapchain->dsv);
    RELEASE(swapchain->ds);
    RELEASE(swapchain->rtv);
    RELEASE(swapchain->tex);

    log("resize %u %u", width, height);
    HRESULT hr = swapchain->swapchain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr)) {
        log("Failed to resize D3D11 swapchain: 0x%X", hr);
        return;
    }

    swapchain->width = width;
    swapchain->height = height;
    createSwapchainBuffers(dev, swapchain);
}
