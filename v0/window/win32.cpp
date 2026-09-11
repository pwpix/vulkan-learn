#include "win32.hpp"
#include <cstddef>
#include <cstdlib>
#include <libloaderapi.h>
#include <minwindef.h>
#include <windef.h>
#include <windows.h>
#include <wingdi.h>


#pragma comment(lib, "dwmapi.lib")


// window procedure

LRESULT CALLBACK WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg) {
    case WM_CREATE:
        break;

    case WM_PAINT: {
        ValidateRect (hWnd, nullptr);
        break;
    }

    case WM_SIZE:
        break;

    case WM_ERASEBKGND: {
        return true;
        break;
    }

    case WM_DESTROY: {
        PostQuitMessage (0);
        break;
    }

    default: {
        return DefWindowProc (hWnd, msg, wParam, lParam);
    }
    }
    return EXIT_SUCCESS;
}

int WIN32_Window_Manager::createWindow (const WindowConfig& config)
{
    this->Hwnd         = nullptr;
    this->HInstance    = GetModuleHandle (nullptr);
    LPCSTR szClassName = "Internal_Window_ClassEx_Name";

    // registering the window class
    WNDCLASSEX wc{ 0 };
    wc.cbSize        = sizeof (WNDCLASSEX);
    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = HInstance;
    wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = szClassName;
    wc.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

    if (!RegisterClassEx (&wc)) {
        MessageBox (NULL, "Window Registration Failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    Hwnd = CreateWindowEx (
    WS_EX_CLIENTEDGE, szClassName, config.windowName,
    WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT, config.windowWidth, config.windowHeight,
    NULL, NULL, HInstance, NULL);

    if (Hwnd == nullptr) {
        MessageBox (NULL, "Window Creation Failed", "Error!",
        MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow (Hwnd, SW_SHOW);
    UpdateWindow (Hwnd);

    return 1;
}

int WIN32_Window_Manager::destroyWindow ()
{
    if (isRunning)
        isRunning = false;
    if (Hrc) {
        // wglDeleteContext(Hrc);
    }
    if (Hdc) {
        ReleaseDC (Hwnd, Hdc);
    }
    if (Hwnd) {
        DestroyWindow (Hwnd);
    }
    Hwnd      = nullptr;
    HInstance = nullptr;
    Hdc       = nullptr;

    return 1;
}

int WIN32_Window_Manager::processMessages ()
{

    if (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
        if (Msg.message == WM_QUIT) {
            return 0;
        }
    }
    return 1;
}

void WIN32_Window_Manager::startWindowLoop ()
{

    if (!isInit)
        return;


    isRunning = true;
    while (isRunning) {

        if (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage (&Msg);
            DispatchMessage (&Msg);
            if (Msg.message == WM_QUIT)
                break;
        }
    }
}
