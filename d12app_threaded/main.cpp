#include "app.h"
#include "draw.h"

static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        if (g_app)
            g_app->releaseResources();
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        if (g_app)
            g_app->render();
    }
        return 0;

    case WM_SIZE:
    {
        const UINT newWidth = UINT(lParam & 0xFFFF);
        const UINT newHeight = UINT(lParam >> 16);
        if (g_app)
            g_app->resize(newWidth, newHeight);
    }
        return 0;

    case WM_MBUTTONDOWN:
        if (g_app) {
            log("Simulating graphics device loss");
            g_app->handleLostDevice();
        }
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

_Use_decl_annotations_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = _T("d12app_threaded");
    if (!RegisterClassEx(&windowClass)) {
        OutputDebugString(_T("RegisterClassEx failed"));
        return EXIT_FAILURE;
    }

    const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    RECT windowRect = { 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT };
    AdjustWindowRect(&windowRect, windowStyle, false);

    HWND window = CreateWindow(windowClass.lpszClassName, _T("d12app_threaded"), windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!window) {
        OutputDebugString(_T("CreateWindow failed"));
        return EXIT_FAILURE;
    }

    ShowWindow(window, nCmdShow);

    App app(hInstance, window, MULTITHREADED ? Builder::ThreadModel::Threaded : Builder::ThreadModel::NonThreaded);
    BuilderHost bldHost;
    app.setFrameFunc(std::bind(&BuilderHost::frame, &bldHost));
    app.addReleaseResourcesFunc(std::bind(&BuilderHost::releaseResourcesNotify, &bldHost));
    app.addPostFrameFunc([&app] { app.requestUpdate(); });

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            app.maybeUpdate();
        }
    }

    return EXIT_SUCCESS;
}
