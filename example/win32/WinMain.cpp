#include "windows/vulkanwindow.hpp"

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
    if (!g_window->Setup("Gaussian Splatting Renderer", hInstance, WndProc, WS_OVERLAPPEDWINDOW)) {
        return 0;
    }
    g_window->RenderLoop();
    delete g_window;
    return 0;
}