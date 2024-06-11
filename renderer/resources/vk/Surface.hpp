#pragma once

#include "VkResourceBase.hpp"
#include "common/VkCommon.hpp"
#include "Image.hpp"

namespace X::Backend {

class Surface : public VkResourceBase {
public:

#ifdef HOST_ANDROID
    [[nodiscard]] static std::unique_ptr<Surface> Make(ANativeWindow* window);
#elif defined HOST_WIN32
    [[nodiscard]] static std::unique_ptr<Surface> Make(void* instance, void* window);
#endif
    vk::SurfaceKHR GetHandle() { return surface_; }
    void SetupSwapchain();
    uint32_t GetPresentQueueIdx() { return presentQueueIdx_; }

protected:
#ifdef HOST_ANDROID
    explicit Surface(ANativeWindow* window);
#elif defined HOST_WIN32
    Surface(void* instance, void* window);
#endif

private:
    void GetImagesFromSwapchain();

private:
    vk::UniqueSurfaceKHR surfaceUnique_;
    vk::SurfaceKHR surface_;

    // TODO: Disable swapchain when use Android, use SurfaceView instead
    vk::UniqueSwapchainKHR swapchainUnique_;
    vk::SwapchainKHR swapchain_;

    uint32_t presentQueueIdx_ = UINT32_MAX;
    uint32_t imageCount_ = 4;
    uint32_t width_ = UINT32_MAX;
    uint32_t height_ = UINT32_MAX;
    std::vector<std::shared_ptr<Image>> images_;
};
    
} // namespace X::Backend
