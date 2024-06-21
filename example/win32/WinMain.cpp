#include "windows/VulkanWindow.hpp"
#include "common/LogCommon.hpp"
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

VulkanWindow* g_window;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (g_window != nullptr) {
        g_window->HandleMessage(uMsg, wParam, lParam);
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR pCmdLine, int nCmdShow)
{
    g_window = new VulkanWindow();
    INIT_LOGGER(plog::debug, "Logs.txt", 5000, 3); // Initialize logging to the file.
    if (!g_window->Setup("Gaussian Splatting Renderer", hInstance, WndProc, WS_OVERLAPPEDWINDOW, WS_OVERLAPPEDWINDOW)) {
        return 0;
    }
    g_window->InitBackend();
    g_window->InitSurface();
    g_window->InitRenderer();
    g_window->LoadScene();
    g_window->RenderLoop();
    delete g_window;
    return 0;
}