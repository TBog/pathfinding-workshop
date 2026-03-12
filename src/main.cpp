#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "App.h"

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                   LPSTR /*lpCmdLine*/, int nCmdShow)
{
    const int screenWidth  = App::GRID_WIDTH  * App::CELL_SIZE;
    const int screenHeight = App::GRID_HEIGHT * App::CELL_SIZE;

    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("PathfindingWorkshop");

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, TEXT("Failed to register window class."),
                   TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    // Calculate the window size so the client area matches the desired resolution
    RECT rect = { 0, 0, screenWidth, screenHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0,
        TEXT("PathfindingWorkshop"),
        TEXT("Pathfinding Workshop  |  1=Wall  2=Start  3=End  4=Erase  C=Clear"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
    {
        MessageBox(NULL, TEXT("Failed to create window."),
                   TEXT("Error"), MB_OK | MB_ICONERROR);
        return -1;
    }

    App* app = new App();
    if (!app->Initialize(hwnd, screenWidth, screenHeight))
    {
        MessageBox(NULL, TEXT("Failed to initialize application.\n"
                              "Ensure the DirectX SDK (June 2010) is installed."),
                   TEXT("Error"), MB_OK | MB_ICONERROR);
        delete app;
        return -1;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    memset(&msg, 0, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            app->Update();
            app->Render();
        }
    }

    app->Shutdown();
    delete app;

    return (int)msg.wParam;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    App* app = reinterpret_cast<App*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (app)
            app->OnKeyDown((int)wParam);
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
        if (app)
            app->OnMouseDown(LOWORD(lParam), HIWORD(lParam), 0);
        return 0;

    case WM_RBUTTONDOWN:
        if (app)
            app->OnMouseDown(LOWORD(lParam), HIWORD(lParam), 1);
        return 0;

    case WM_MOUSEMOVE:
        if (app)
            app->OnMouseMove(LOWORD(lParam), HIWORD(lParam), (int)wParam);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
