#pragma once

#include <cassert>
#ifdef _DEBUG
#include <cstdio>
#endif
#include <minwindef.h>
#include <windef.h>
#include <windows.h>


LRESULT CALLBACK WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class WIN32_Window_Manager;

struct WindowConfig {

    explicit WindowConfig (
    const char* str /*window name*/,
    int w /*window width*/,
    int h /*window height*/
    ) : windowName (str), windowWidth (w), windowHeight (h)
    {
    }

    const char* windowName;
    int windowHeight;
    int windowWidth;
};


class WIN32_Window_Manager {

    public:
    WIN32_Window_Manager () = default;
    ~WIN32_Window_Manager ()
    {
        cleanup ();
    }

    void initializeWindowManager (const WindowConfig& config)
    {

#ifdef _DEBUG
        printf ("initialising window manager\n");
#endif

        if (createWindow (config)) {
            Hdc          = GetDC (Hwnd);
            this->isInit = true;
#ifdef _DEBUG
            printf ("window init success\n");
#endif
        }
        assert (isInit == true);
        HInstance = GetModuleHandle (nullptr);
    }

    void startWindowLoop ();

    void cleanup ()
    {
        if (!isInit)
            return;

#ifdef _DEBUG
        printf ("destroy window context\n");
#endif
        assert (isInit == true);
        auto res = destroyWindow ();
#ifdef _DEBUG
        assert (res == 1);
        printf ("destroy window context success\n");
#endif
        isInit = false;
    }

    HWND getWindowHandle ()
    {
        return Hwnd;
    }

    HINSTANCE getInstance ()
    {
        return HInstance;
    }


    [[maybe_unused]] bool getWindowSize (int* width, int* height)
    {
        RECT rect;
        if (GetClientRect (Hwnd, &rect)) {
            *width  = (int)rect.bottom - rect.left;
            *height = (int)rect.bottom - rect.top;
            return true;
        }
        return false;
    }


    private:
    int createWindow (const WindowConfig& config);
    int destroyWindow ();

    private:
    HWND Hwnd           = nullptr;
    HINSTANCE HInstance = nullptr;
    MSG Msg;
    HDC Hdc         = nullptr;
    HGLRC Hrc       = nullptr;
    HMODULE hModule = nullptr;

    bool isInit    = false;
    bool isRunning = false;
    bool isFocused = false;
};
