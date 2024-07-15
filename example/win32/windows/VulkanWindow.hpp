#pragma once

#include "BaseWindow.hpp"

#include <ctime>
#include <chrono>

#include "GaussianRenderer.hpp"
#include "scenes/Scene.hpp"
#include "resources/vk/DisplaySurface.hpp"

class VulkanWindow : public BaseWindow<VulkanWindow> {
public:
    LPCSTR ClassName() const override { return "Vulkan Window Class"; }
    std::string GetWindowTitle() const override { return ClassName(); }
    void Init();
    void HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void RenderLoop();

protected:
    friend class BaseWindow<VulkanWindow>;
    void HandleMouseMove(int32_t x, int32_t y);
    void OnHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {};
    void WindowResize() override;

    void InitBackend();
    void InitRenderer();
    void InitSurface();
    void LoadScene();

private:
    std::unique_ptr<X::Renderer> renderer_;
    std::unique_ptr<X::Scene> scene_;
    std::unique_ptr<X::Backend::DisplaySurface> surface_;

    // State of mouse input
    struct {
        struct {
            bool left = false;
            bool right = false;
            bool middle = false;
        } buttons_;
        glm::vec2 position_;
    } mouseState_;
    bool resizing_ = false;

    // TODO: create a time statistics class
    std::chrono::time_point<std::chrono::high_resolution_clock> frameStart_;
};