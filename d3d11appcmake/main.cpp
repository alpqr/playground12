#include "app.h"
#include "frame.h"
#include "keymap.h"

void log(const char *fmt, ...)
{
    char newLineFmt[512];
    strncpy_s(newLineFmt, sizeof(newLineFmt) - 1, fmt, _TRUNCATE);
    strcat_s(newLineFmt, sizeof(newLineFmt), "\n");

    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(msg, sizeof(msg), _TRUNCATE, newLineFmt, args);
    va_end(args);

    OutputDebugStringA(msg);
}

Event Event::mouseEvent(EventType type, LPARAM lParam)
{
    Event e = {};
    e.type = type;
    e.x = GET_X_LPARAM(lParam);
    e.y = GET_Y_LPARAM(lParam);
    e.modifiers = currentModifiers();
    return e;
}

Event Event::keyEvent(EventType type, WPARAM wParam, LPARAM lParam)
{
    Event e = {};
    e.type = type;
    e.key = keyMap[wParam & 0xFF];
    e.modifiers = currentModifiers();
    if (type == EventType::KeyDown)
        e.repeat = (lParam & 0x40000000) != 0;
    return e;
}

Event Event::charEvent(WPARAM wParam, LPARAM lParam)
{
    Event e = {};
    e.type = EventType::Char;
    e.ch = uint32_t(wParam);
    e.modifiers = currentModifiers();
    e.repeat = (lParam & 0x40000000) != 0;
    return e;
}

int Event::currentModifiers()
{
    int m = 0;
    const int downBit = 1 << 15;
    if (GetKeyState(VK_SHIFT) & downBit)
        m |= Event::Shift;
    if (GetKeyState(VK_CONTROL) & downBit)
        m |= Event::Ctrl;
    if (GetKeyState(VK_MENU) & downBit)
        m |= Event::Alt;
    if (GetKeyState(VK_LWIN) & downBit)
        m |= Event::Super;
    if (GetKeyState(VK_RWIN) & downBit)
        m |= Event::Super;
    return m;
}

static App app;

bool initApp(HWND window)
{
    if (!createDevice(&app.dev))
        return false;

    if (!createSwapchain(&app.dev, window, DEFAULT_WIDTH, DEFAULT_HEIGHT, &app.swapchain))
        return false;

    app.renderable = true;

    return true;
}

void destroyApp()
{
    destroySwapchain(&app.swapchain);
    destroyDevice(&app.dev);
}

void App::event(const Event &e)
{
    handleEvent(e);
}

void App::render()
{
    if (!renderable)
        return;

    recordFrame(this);

    HRESULT hr = swapchain.swapchain->Present(1, 0);
    if (FAILED(hr))
        log("Present failed: 0x%X", hr);
}

static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        destroyApp();
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        app.render();
    }
        return 0;

    case WM_SIZE:
    {
        const uint32_t newWidth = (uint32_t) (lParam & 0xFFFF);
        const uint32_t newHeight = (uint32_t) (lParam >> 16);
        if (newWidth > 0 && newHeight > 0 && wParam != SIZE_MINIMIZED) {
            app.renderable = true;
            if (app.swapchain.swapchain)
                resizeSwapchain(&app.dev, &app.swapchain, newWidth, newHeight);
        } else {
            app.renderable = false;
        }
    }
        return 0;

    case WM_LBUTTONDOWN:
        app.event(Event::mouseEvent(EventType::MouseLeftDown, lParam));
        break;
    case WM_LBUTTONUP:
        app.event(Event::mouseEvent(EventType::MouseLeftUp, lParam));
        break;
    case WM_RBUTTONDOWN:
        app.event(Event::mouseEvent(EventType::MouseRightDown, lParam));
        break;
    case WM_RBUTTONUP:
        app.event(Event::mouseEvent(EventType::MouseRightUp, lParam));
        break;
    case WM_MBUTTONDOWN:
        app.event(Event::mouseEvent(EventType::MouseMiddleDown, lParam));
        break;
    case WM_MBUTTONUP:
        app.event(Event::mouseEvent(EventType::MouseMiddleUp, lParam));
        break;
    case WM_MOUSEMOVE:
        app.event(Event::mouseEvent(EventType::MouseMove, lParam));
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        app.event(Event::keyEvent(EventType::KeyDown, wParam, lParam));
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        app.event(Event::keyEvent(EventType::KeyUp, wParam, lParam));
        break;
    case WM_CHAR:
        app.event(Event::charEvent(wParam, lParam));
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

_Use_decl_annotations_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX windowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = windowProc,
        .hInstance = hInstance,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = "D3D11APP"
    };
    if (!RegisterClassExA(&windowClass)) {
        log("RegisterClassEx failed");
        return EXIT_FAILURE;
    }

    const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    RECT windowRect = { 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT };
    AdjustWindowRect(&windowRect, windowStyle, FALSE);

    HWND window = CreateWindowA(windowClass.lpszClassName, "d3d11app",
                                windowStyle,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                                NULL, NULL, hInstance, NULL);
    if (!window) {
        log("CreateWindow failed");
        return EXIT_FAILURE;
    }

    ShowWindow(window, nCmdShow);

    app = {};
    if (!initApp(window))
        return EXIT_FAILURE;

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            app.maybeRender();
        }
    }

    return EXIT_SUCCESS;
}
