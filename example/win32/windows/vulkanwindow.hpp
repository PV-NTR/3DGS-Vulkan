#pragma once

#include "basewindow.hpp"

#include "renderer.hpp"
#include "scenes/scene.hpp"

class VulkanWindow : public BaseWindow<VulkanWindow> {
public:
    LPCSTR ClassName() const override { return "Vulkan Window Class"; }
	std::string GetWindowTitle() const override { return ClassName(); }
	void HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void RenderLoop() { };
	void InitRenderer();
	void LoadScene();

protected:
	friend class BaseWindow<VulkanWindow>;
	void HandleMouseMove(int32_t x, int32_t y);
	void OnHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {};
	void WindowResize() override {};

private:
    X::Renderer renderer_;
    X::Scene scene_;

	// State of mouse/touch input
	struct {
		struct {
			bool left = false;
			bool right = false;
			bool middle = false;
		} buttons_;
		glm::vec2 position_;
	} mouseState_;
    bool resizing_ = false;
};