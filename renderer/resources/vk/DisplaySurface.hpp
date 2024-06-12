#pragma once

#include "Surface.hpp"

namespace X::Backend {

class DisplaySurface : public Surface {
public:

#ifdef HOST_ANDROID
    [[nodiscard]] static std::unique_ptr<DisplaySurface> Make(ANativeWindow* window);
#elif defined HOST_WIN32
    [[nodiscard]] static std::unique_ptr<DisplaySurface> Make(void* instance, void* window);
#endif
    vk::SurfaceKHR GetHandle() const { return surface_; }
    void SetupSwapchain();
    void SetupFramebuffer(std::shared_ptr<RenderPass> renderPass, std::shared_ptr<Image> depthImage = nullptr);
    void Present();
    uint32_t GetPresentQueueIdx() const { return presentQueueIdx_; }
    uint32_t GetCurrentFrameIdx() const { return currentFrame_; }
    uint32_t GetSwapbufferCount() const { return imageCount_; }
    vk::Semaphore GetAcquireFrameSignalSemaphore() { return acquireFrameSignalSemaphore_; };
    vk::Semaphore GetPresentWaitSemaphore() { return presentWaitSemaphore_; };
    uint32_t NextFrame();

protected:
#ifdef HOST_ANDROID
    explicit DisplaySurface(ANativeWindow* window);
#elif defined HOST_WIN32
    DisplaySurface(void* instance, void* window);
#endif

private:
    void GetImagesFromSwapchain();

private:
    vk::UniqueSurfaceKHR surfaceUnique_;
    vk::SurfaceKHR surface_;

    // TODO: Disable swapchain when use Android, use SurfaceView instead
    Swapchain swapchain_;
    std::vector<std::shared_ptr<Image>> images_;
    std::vector<std::shared_ptr<Image>> depthImages_;
    vk::Semaphore acquireFrameSignalSemaphore_;
    vk::Semaphore presentWaitSemaphore_;

    uint32_t presentQueueIdx_ = UINT32_MAX;
    uint32_t currentFrame_ = 0;
    uint32_t imageCount_ = 4;
};
    
} // namespace X::Backend
