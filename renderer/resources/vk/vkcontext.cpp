#include "vkcontext.hpp"

#ifdef HOST_ANDROID
#include <dlfcn.h>
#elif defined HOST_WIN32
#include <windows.h>
#include <libloaderapi.h>
#endif
namespace X::Backend {

std::vector<const char*> VkContext::requiredInstanceExts = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef HOST_ANDROID
    "VK_KHR_android_surface"
#elif defined HOST_WIN32
    "VK_KHR_win32_surface"
#endif
};
std::vector<const char*> VkContext::requiredInstanceLayers {
    "VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> VkContext::requiredDeviceExts {
#ifdef HOST_WIN32
    "VK_KHR_swapchain"
#endif
};

void VkContext::Init()
{
    LoadVkLibrary();

    CreateInstance();
    SelectPhysicalDevice();
    CreateDeviceAndQueues();

    InitAllocator();
}

void VkContext::LoadVkLibrary()
{
#ifdef HOST_ANDROID
    auto handle = dlopen("/system/lib64/libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    entryFunc_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(handle, "vkGetInstanceProcAddr"));
#elif defined HOST_WIN32
    auto handle = LoadLibraryA("vulkan-1.dll");
    entryFunc_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(handle, "vkGetInstanceProcAddr"));
#endif
    // why no defination?
     VULKAN_HPP_DEFAULT_DISPATCHER.init(entryFunc_);
}

void VkContext::CreateInstance()
{
    if (!entryFunc_) {
        return;
    }
}

void VkContext::SelectPhysicalDevice()
{

}

void VkContext::CreateDeviceAndQueues()
{

}

void VkContext::InitAllocator()
{

}

bool VkContext::IsReady()
{
    return entryFunc_ && instance_ && physicalDevice_ && device_;
}

} // namespace X::Backend