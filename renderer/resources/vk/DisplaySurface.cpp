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
    surface_ = *surfaceUnique_;
    presentQueueIdx_ = VkContext::GetInstance().QueryPresentQueueFamilies(surface_);
    if (presentQueueIdx_ == UINT32_MAX) {
        XLOGE("No valid present queue!");
    }
    screenSize_ = Backend::VkResourceManager::GetInstance().GetBufferManager()
        .RequireBuffer({ 8, BufferType::Uniform });
    InitDisplaySemaphores();
}

void DisplaySurface::InitDisplaySemaphores()
{
    acquireFrameSignalSemaphore_ = VkContext::GetInstance().GetDevice().createSemaphore({}).value;
    presentWaitSemaphore_ = VkContext::GetInstance().GetDevice().createSemaphore({}).value;
    assert(acquireFrameSignalSemaphore_ && presentWaitSemaphore_);
}

void DisplaySurface::CleanSwapchain()
{
    ready_ = false;
    swapchain_.reset();
    // swapSurfaces_.clear();
    if (depthStencil_) {
        depthStencil_.reset();
    }
}

void DisplaySurface::SetupSwapchain()
{
    changed_ = true;
    // WARNING: check result
    vk::SurfaceCapabilitiesKHR surfCaps = VkContext::GetInstance().GetPhysicalDevice()
        .getSurfaceCapabilitiesKHR(surface_).value;

    assert(surfCaps.currentExtent.width != UINT32_MAX);
    assert(surfCaps.currentExtent.height != UINT32_MAX);
    swapchainImageInfo_.width_ = surfCaps.currentExtent.width;
    swapchainImageInfo_.height_ = surfCaps.currentExtent.height;

    imageCount_ = std::max(std::min(imageCount_, surfCaps.maxImageCount), surfCaps.minImageCount);

    // Find a format for surface
    // WARNING: check result
    std::vector<vk::SurfaceFormatKHR> formats = VkContext::GetInstance().GetPhysicalDevice()
        .getSurfaceFormatsKHR(surface_).value;
    for (const auto& format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Unorm || format.format == vk::Format::eB8G8R8A8Unorm ||
            format.format == vk::Format::eA8B8G8R8UnormPack32) {
            swapchainImageInfo_.format_ = format.format;
            swapchainColorSpace_ = format.colorSpace;
            break;
        }
    }
    assert(swapchainImageInfo_.format_ != vk::Format::eUndefined);

    // Find a supported composite alpha format (not all devices support alpha opaque)
    vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    // Simply select the first composite alpha format available
    std::vector<vk::CompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
        vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
        vk::CompositeAlphaFlagBitsKHR::eInherit,
    };
    for (auto& compositeAlphaFlag : compositeAlphaFlags) {
        if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
        };
    }

    vk::SwapchainCreateInfoKHR swapchainCI {};
    // TODO: can unique handle set old swapchain?
    swapchainCI.setSurface(surface_)
        .setMinImageCount(imageCount_)
        .setImageFormat(swapchainImageInfo_.format_)
        .setImageColorSpace(swapchainColorSpace_)
        .setImageExtent({ swapchainImageInfo_.width_, swapchainImageInfo_.height_ })
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setQueueFamilyIndices(presentQueueIdx_)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setCompositeAlpha(compositeAlpha)
        .setPresentMode(vk::PresentModeKHR::eFifo)
        .setClipped(vk::True);
        //.setOldSwapchain(*swapchain_);

    auto [ret, swapchainUnique] = VkContext::GetInstance().GetDevice().createSwapchainKHRUnique(swapchainCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateSwapchain failed, errCode: %d", ret);
        return;
    }
    swapchain_.swap(swapchainUnique);
}

std::vector<std::shared_ptr<Image>> DisplaySurface::GetImagesFromSwapchain()
{
    if (*swapchain_ == VK_NULL_HANDLE) {
        return {};
    }
    auto [ret, vkImages] = VkContext::GetInstance().GetDevice().getSwapchainImagesKHR(*swapchain_);
    if (ret != vk::Result::eSuccess) {
        XLOGW("GetSwapchainImages failed, errCode: %d", ret);
    }
    imageCount_ = vkImages.size();
    // assert(vkImages.size() == imageCount_);

    std::vector<std::shared_ptr<Image>> images;
    images.reserve(imageCount_);
    for (auto&& vkImage : vkImages) {
        images.emplace_back(new Image(vkImage, swapchainImageInfo_, {
            vk::ImageLayout::eUndefined, vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eBottomOfPipe }));
        images.back()->CreateView();
    }
    return images;
}

uint32_t DisplaySurface::NextFrame()
{
    if (*swapchain_ == VK_NULL_HANDLE) {
        return currentFrame_;
    }
    auto ret = VkContext::GetInstance().GetDevice().waitIdle();
    assert(ret != vk::Result::eErrorDeviceLost);
    auto nextIndex = VkContext::GetInstance().GetDevice()
        .acquireNextImageKHR(*swapchain_, UINT64_MAX, acquireFrameSignalSemaphore_, {});
    currentFrame_ = nextIndex.value;
    return currentFrame_;
}

void DisplaySurface::Present()
{
    X::Backend::VkContext::GetInstance().GetDevice().waitIdle();
    auto queue = VkContext::GetInstance().AcquireGraphicsQueue(presentQueueIdx_);
    std::array<vk::Semaphore, 1> waitSemaphores { presentWaitSemaphore_ };
    vk::PresentInfoKHR presentInfo{};
    presentInfo.setImageIndices(currentFrame_)
        .setSwapchains(*swapchain_)
        .setWaitSemaphores(waitSemaphores);
    auto ret = queue.presentKHR(presentInfo);
    if (ret == vk::Result::eSuboptimalKHR || ret == vk::Result::eErrorOutOfDateKHR) {
        XLOGW("Swapchain need changing, retCode: %d", ret);
        changed_ = true;
        auto waitRet = X::Backend::VkContext::GetInstance().GetDevice().waitIdle();
        if (waitRet != vk::Result::eSuccess) {
            XLOGW("waitIdle failed, errCode: %d", waitRet);
        }
        CleanSwapchain();
        SetupSwapchain();
        SetupSwapSurfaces();
        return;
    } else if (ret != vk::Result::eSuccess) {
        XLOGE("Present failed, errCode: %d", ret);
    }
    NextFrame();
}

void DisplaySurface::SetupSwapSurfaces(bool enableDepthStencil)
{
    swapSurfaces_.clear();
    enableDepthStencil_ = enableDepthStencil;
    auto images = GetImagesFromSwapchain();
    if (enableDepthStencil) {
        depthStencil_ = VkResourceManager::GetInstance().GetImageManager().RequireImage(
            { swapchainImageInfo_.width_, swapchainImageInfo_.height_, vk::Format::eD32SfloatS8Uint, true });
    } else {
        depthStencil_.reset();
    }
    std::vector<std::shared_ptr<Image>> attachmentResources(2);
    attachmentResources[1] = depthStencil_;
    auto renderPass = RenderPass::MakeDisplay(swapchainImageInfo_.format_, enableDepthStencil_);
    for (const auto& image : images) {
        attachmentResources[0] = image;
        auto swapSurface = Surface::Make(renderPass, attachmentResources);
        swapSurface->Init();
        swapSurfaces_.emplace_back(std::move(swapSurface));
    }
    NextFrame();
    ready_ = true;
}

void DisplaySurface::UpdateScreenSizeBuffer()
{
    if (changed_) {
        float data[2] = {
            static_cast<float>(swapchainImageInfo_.width_), static_cast<float>(swapchainImageInfo_.height_) };
        screenSize_->Update(data, 8, 0);
        changed_ = false;
    }
}

} // namespace X::Backend
