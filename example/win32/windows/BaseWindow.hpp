#pragma once
#include <windows.h>
#include <cstdint>
#include <iostream>
#include <string>

template <class DERIVED>
class BaseWindow {
public:
    BaseWindow() : hWnd_(NULL) { }

    BOOL Setup(LPCSTR lpWindowName, HINSTANCE hInstance, WNDPROC wndproc,
        int nWidth = CW_USEDEFAULT, int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0, HMENU hMenu = 0)
    {
        hInstance_ = hInstance;
        width_ = nWidth;
        height_ = nHeight;

        WNDCLASSEX wndClass;

        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = wndproc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = ClassName();
        wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

        if (!RegisterClassEx(&wndClass)) {
            std::cout << "Could not register window class!\n";
            fflush(stdout);
            exit(1);
        }

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        if (fullscreen_) {
            if ((width_ != (uint32_t)screenWidth) && (height_ != (uint32_t)screenHeight)) {
                DEVMODE dmScreenSettings;
                memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
                dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
                dmScreenSettings.dmPelsWidth  = width_;
                dmScreenSettings.dmPelsHeight = height_;
                dmScreenSettings.dmBitsPerPel = 32;
                dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
                if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL){
                    if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error",
                        MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
                        fullscreen_ = false;
                    } else {
                        return false;
                    }
                }
                screenWidth = width_;
                screenHeight = height_;
            }
        }

        DWORD dwExStyle;
        DWORD dwStyle;

        if (fullscreen_) {
            dwExStyle = WS_EX_APPWINDOW;
            dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        } else {
            dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
            dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        }

        RECT windowRect;
        windowRect.left = 0L;
        windowRect.top = 0L;
        windowRect.right = fullscreen_ ? (long)screenWidth : (long)width_;
        windowRect.bottom = fullscreen_ ? (long)screenHeight : (long)height_;

        AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

        std::string windowTitle = GetWindowTitle();
        hWnd_ = CreateWindowEx(0,
            ClassName(),
            windowTitle.c_str(),
            dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0,
            0,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            NULL,
            NULL,
            hInstance_,
            NULL);

        if (!fullscreen_) {
            // Center on screen
            uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
            uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
            SetWindowPos(hWnd_, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }

        if (!hWnd_) {
            printf("Could not create window!\n");
            fflush(stdout);
            return false;
        }

        ShowWindow(hWnd_, SW_SHOW);
        SetForegroundWindow(hWnd_);
        SetFocus(hWnd_);
        return true;
    }

    HWND Window() const { return hWnd_; }

protected:
    virtual LPCSTR ClassName() const = 0;
    virtual void HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual std::string GetWindowTitle() const = 0;
    virtual void WindowResize() = 0;

    HWND hWnd_;
    HINSTANCE hInstance_;
    uint32_t width_;
    uint32_t height_;
    bool fullscreen_ = false;
};
