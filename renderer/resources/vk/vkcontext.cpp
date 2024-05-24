#include "vkcontext.hpp"

#ifdef HOST_ANDROID
#include <dlfcn.h>
#elif defined HOST_WIN32
#include <libloaderapi.h>
#endif
namespace X::Backend {

void VkContext::Init()
{
    LoadLibrary();

    CreateInstance();
    SelectPhysicalDevice();
    CreateDeviceAndQueues();

    InitAllocator();
}

void VkContext::LoadLibrary()
{
#ifdef HOST_ANDROID
    auto handle = dlopen("/system/lib64/libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    entryFunc_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(handle, "vkGetInstanceProcAddr"));
#elif defined HOST_WIN32
    auto handle = LoadLibraryA("vulkan-1.dll");
    entryFunc_ = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(handle, "vkGetInstanceProcAddr"));
#endif
    // why no defination?
    // VULKAN_HPP_DEFAULT_DISPATCHER.init(entryFunc_);
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