#include "DisplaySurface.hpp"

#include "VkContext.hpp"
#include "VkResourceManager.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

#ifdef HOST_ANDROID
std::unique_ptr<DisplaySurface> DisplaySurface::Make(ANativeWindow* window)
{
    return std::unique_ptr<DisplaySurface>(new DisplaySurface(window));
}
#elif defined HOST_WIN32
std::unique_ptr<DisplaySurface> DisplaySurface::Make(void* instance, void* window)
{
    return std::unique_ptr<DisplaySurface>(new DisplaySurface(instance, window));
}
#endif

#ifdef HOST_ANDROID
DisplaySurface::DisplaySurface(ANativeWindow* window)
#elif defined HOST_WIN32
DisplaySurface::DisplaySurface(void* instance, void* window)
#endif
{
#ifdef HOST_ANDROID
    vk::AndroidSurfaceCreateInfoKHR surfaceCI {};
    surfaceCI.setWindow(window);
    auto [ret, surfaceUnique] = VkContext::GetInstance().instance_.createAndroidSurfaceKHRUnique(surfaceCI);
#elif defined HOST_WIN32
    vk::Win32SurfaceCreateInfoKHR surfaceCI {};
    surfaceCI.setHinstance(reinterpret_cast<HINSTANCE>(instance))
        .setHwnd(reinterpret_cast<HWND>(window));
    auto [ret, surfaceUnique] = VkContext::GetInstance().GetVkInstance().createWin32SurfaceKHRUnique(surfaceCI);
#endif
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateSurface failed, errCode: %d", ret);
        return;
    }
    surfaceUnique_.swap(surfaceUnique);
    surface_ = *surfaceUnique;
    presentQueueIdx_ = VkContext::GetInstance().QueryPresentQueueFamilies(surface_);
    if (presentQueueIdx_ == UINT32_MAX) {
        XLOGE("No valid present queue!");
    }
    screenSize_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ 2, BufferType::Uniform });
}

void DisplaySurface::SetupSwapchain()
{
    resized_ = true;
    // WARNING: check result
    vk::SurfaceCapabilitiesKHR surfCaps = VkContext::GetInstance().GetPhysicalDevice().getSurfaceCapabilitiesKHR(surface_).value;

    assert(surfCaps.currentExtent.width != UINT32_MAX);
    assert(surfCaps.currentExtent.height != UINT32_MAX);
    width_ = surfCaps.currentExtent.width;
    height_ = surfCaps.currentExtent.height;

    imageCount_ = std::max(std::min(imageCount_, surfCaps.maxImageCount), surfCaps.minImageCount);

    vk::SwapchainCreateInfoKHR swapchainCI {};
    // TODO: can unique handle set old swapchain?
    swapchainCI.setSurface(surface_)
        .setMinImageCount(imageCount_)
        .setImageFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
        .setImageExtent({ width_, height_ })
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setQueueFamilyIndices(presentQueueIdx_)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit)
        .setPresentMode(vk::PresentModeKHR::eFifo)
        .setClipped(vk::True)
        .setOldSwapchain(*swapchain_);

    auto [ret, swapchainUnique] = VkContext::GetInstance().GetDevice().createSwapchainKHRUnique(swapchainCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateSwapchain failed, errCode: %d", ret);
        return;
    }
    swapchain_.swap(swapchainUnique);
}

std::vector<std::shared_ptr<Image>> DisplaySurface::GetImagesFromSwapchain()
{
    // WARNING: check return
    auto vkImages = VkContext::GetInstance().GetDevice().getSwapchainImagesKHR(*swapchain_).value;
    assert(vkImages.size() == imageCount_);

    std::vector<std::shared_ptr<Image>> images;
    images.reserve(imageCount_);
    for (auto&& vkImage : vkImages) {
        images.emplace_back(new Image(vkImage, ImageInfo{ width_, height_ }));
    }
    return images;
}

uint32_t DisplaySurface::NextFrame()
{
    auto nextIndex = VkContext::GetInstance().GetDevice().acquireNextImageKHR(*swapchain_, UINT64_MAX, acquireFrameSignalSemaphore_, {});
    currentFrame_ = nextIndex.value;
    return currentFrame_;
}

void DisplaySurface::Present()
{
    auto queue = VkContext::GetInstance().AcquireGraphicsQueue(presentQueueIdx_);
    vk::PresentInfoKHR presentInfo{};
    presentInfo.setImageIndices(currentFrame_)
        .setSwapchains(*swapchain_)
        .setWaitSemaphores(presentWaitSemaphore_);
    queue.presentKHR(presentInfo);
}

void DisplaySurface::SetupSwapSurfaces(bool enableDepthStencil)
{
    enableDepthStencil_ = enableDepthStencil;
    auto images = GetImagesFromSwapchain();
    if (enableDepthStencil) {
        depthStencil_ = VkResourceManager::GetInstance().GetImageManager().RequireImage({ width_, height_, vk::Format::eD32SfloatS8Uint });
    } else {
        depthStencil_ = nullptr;
    }
    std::vector<std::shared_ptr<Image>> attachmentResources(2);
    attachmentResources[1] = depthStencil_;
    for (const auto& image : images) {
        attachmentResources[0] = image;
        auto swapSurface = Surface::Make(attachmentResources);
        swapSurface->Init();
        swapSurfaces_.emplace_back(std::move(swapSurface));
    }
}

void DisplaySurface::UpdateScreenSizeBuffer()
{
    if (resized_) {
        uint32_t data[2] = { width_, height_ };
        screenSize_->Update(data, 2, 0);
        resized_ = false;
    }
}

} // namespace X::Backend
