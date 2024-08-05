#include "windows/VulkanWindow.hpp"
#include "common/LogCommon.hpp"

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
    INIT_LOGGER(plog::debug, "Logs.txt", 5000, 3); // Initialize logging to the file.

    g_window = new VulkanWindow();
    if (!g_window->Setup("Gaussian Splatting Renderer", hInstance, WndProc, 1959, 1090)) {
        return 0;
    }
    g_window->Init();
    g_window->RenderLoop();
    delete g_window;
    return 0;
}