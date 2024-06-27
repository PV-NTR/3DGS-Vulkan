#include "VkContext.hpp"

#ifdef HOST_ANDROID
#include <dlfcn.h>
#elif defined HOST_WIN32
#include <windows.h>
#include <libloaderapi.h>
#endif

#include <algorithm>
#include "common/LogCommon.hpp"

namespace X::Backend {

std::vector<const char*> VkContext::requiredInstanceExts_ = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef HOST_ANDROID
    "VK_KHR_android_surface"
#elif defined HOST_WIN32
    "VK_KHR_win32_surface"
#endif
};
std::vector<const char*> VkContext::requiredInstanceLayers_ = {
    "VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> VkContext::requiredDeviceExts_ = {
#ifdef HOST_WIN32
    "VK_KHR_swapchain",
    "VK_NV_device_diagnostic_checkpoints",
#endif
    "VK_KHR_buffer_device_address",
    "VK_KHR_synchronization2",
    "VK_EXT_descriptor_indexing",
    "VK_KHR_timeline_semaphore"
};

void VkContext::Init()
{
    LoadVkLibrary();

    CreateInstance();
    SelectPhysicalDevice();
    QueryQueueFamilies();
    CreateDeviceAndQueues();

    InitAllocator();
    CreateCmdPools();
}

bool VkContext::IsReady()
{
    return entryFunc_ && instance_ && physicalDevice_ && device_;
}

vk::Queue VkContext::AcquireGraphicsQueue(uint32_t idx)
{
    return device_.getQueue(graphicsQueueFamilyIdx_, idx);
}

vk::Queue VkContext::AcquireComputeQueue(uint32_t idx)
{
    return device_.getQueue(computeQueueFamilyIdx_, idx);
}

std::pair<uint32_t, vk::Queue> VkContext::AcquireCurrentGraphicsQueue()
{
    return { graphicsQueueIdx_, device_.getQueue(graphicsQueueFamilyIdx_, graphicsQueueIdx_) };
}

std::pair<uint32_t, vk::Queue> VkContext::AcquireCurrentComputeQueue()
{
    return { computeQueueIdx_, device_.getQueue(computeQueueFamilyIdx_, computeQueueIdx_) };
}

std::pair<uint32_t, vk::Queue> VkContext::AcquireNextGraphicsQueue()
{
    graphicsQueueIdx_.fetch_add(1);
    return { graphicsQueueIdx_, device_.getQueue(graphicsQueueFamilyIdx_, graphicsQueueIdx_) };
}

std::pair<uint32_t, vk::Queue> VkContext::AcquireNextComputeQueue()
{
    computeQueueIdx_.fetch_add(1);
    return { computeQueueIdx_, device_.getQueue(computeQueueFamilyIdx_, computeQueueIdx_) };
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
    VULKAN_HPP_DEFAULT_DISPATCHER.init(entryFunc_);
}

void VkContext::GetSupportedInstanceExtensions()
{
    // Get extensions supported by the instance and store for later use
    uint32_t extCount = 0;
    (void)vk::enumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<vk::ExtensionProperties> extensions(extCount);
        if (vk::enumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == vk::Result::eSuccess) {
            for (const vk::ExtensionProperties& extension : extensions) {
                supportedInstanceExts_.emplace_back(extension.extensionName.operator std::string());
            }
        }
    }
}

void VkContext::CollectEnabledInstanceExtensions()
{
    // Enabled requested instance extensions
    if (requiredInstanceExts_.size() > 0) {
        for (const char* requiredInstanceExt : requiredInstanceExts_) {
            // Output message if requested extension is not available
            if (std::find(supportedInstanceExts_.begin(), supportedInstanceExts_.end(), requiredInstanceExt) ==
                supportedInstanceExts_.end()) {
                XLOGE("Required instance extension %s is not supported!", requiredInstanceExt);
            } else {
                enabledInstanceExts_.push_back(requiredInstanceExt);
            }
        }
    }
}

void VkContext::GetSupportedInstanceLayers()
{
    // Get layers supported by the instance and store for later use
    uint32_t layerCount = 0;
    (void)vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount > 0) {
        std::vector<vk::LayerProperties> layers(layerCount);
        if (vk::enumerateInstanceLayerProperties(&layerCount, &layers.front()) == vk::Result::eSuccess) {
            for (const vk::LayerProperties& layer : layers) {
                supportedInstanceLayers_.push_back(layer.layerName.operator std::string());
            }
        }
    }
}

void VkContext::CollectEnabledInstanceLayers()
{
    // Enabled requested instance layers
    if (requiredInstanceLayers_.size() > 0) {
        for (const char* requiredInstancelayer : requiredInstanceLayers_) {
            // Output message if requested extension is not available
            if (std::find(supportedInstanceLayers_.begin(), supportedInstanceLayers_.end(), requiredInstancelayer) ==
                supportedInstanceLayers_.end()) {
                XLOGE("Required instance layer %s is not supported!", requiredInstancelayer);
            } else {
                enabledInstanceLayers_.push_back(requiredInstancelayer);
            }
        }
    }
}

void VkContext::GetSupportedDeviceExtensions()
{
    assert(physicalDevice_ != VK_NULL_HANDLE);
    // Get extensions supported by the device and store for later use
    uint32_t extCount = 0;
    (void)physicalDevice_.enumerateDeviceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<vk::ExtensionProperties> extensions(extCount);
        if (physicalDevice_.enumerateDeviceExtensionProperties(nullptr, &extCount, &extensions.front()) == vk::Result::eSuccess) {
            for (const vk::ExtensionProperties& extension : extensions) {
                supportedDeviceExts_.push_back(extension.extensionName);
            }
        }
    }
}

void VkContext::CollectEnabledDeviceExtensions()
{
    // Enabled requested instance extensions
    if (requiredDeviceExts_.size() > 0) {
        for (const char* requiredDeviceExt : requiredDeviceExts_) {
            // Output message if requested extension is not available
            if (std::find(supportedDeviceExts_.begin(), supportedDeviceExts_.end(), requiredDeviceExt) ==
                supportedDeviceExts_.end()) {
                XLOGE("Required device extension %s is not supported!", requiredDeviceExt);
            } else {
                enabledDeviceExts_.push_back(requiredDeviceExt);
            }
        }
    }
}

bool VkContext::CreateInstance()
{
    if (!entryFunc_) {
        XLOGE("No EntryFunction!");
        return false;
    }
    GetSupportedInstanceExtensions();
    CollectEnabledInstanceExtensions();
    GetSupportedInstanceLayers();
    CollectEnabledInstanceLayers();

    vk::InstanceCreateInfo instanceCI {};
    instanceCI.setEnabledExtensionCount(enabledInstanceExts_.size())
        .setPpEnabledExtensionNames(enabledInstanceExts_.data())
        .setEnabledLayerCount(enabledInstanceLayers_.size())
        .setPpEnabledLayerNames(enabledInstanceLayers_.data());

    vk::ApplicationInfo appInfo {};
    appInfo.setApiVersion(VK_API_VERSION_1_3);
    instanceCI.setPApplicationInfo(&appInfo);

    auto [ret, instance] = vk::createInstanceUnique(instanceCI, nullptr);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateInstance failed, errCode: %d", ret);
        return false;
    }
    uniqueInstance_.swap(instance);
    instance_ = *uniqueInstance_;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
    return true;
}

bool VkContext::SelectPhysicalDevice()
{
    if (!instance_) {
        XLOGE("Instance not initialized!");
        return false;
    }
    auto [ret, physicalDevices] = instance_.enumeratePhysicalDevices();
    if (ret != vk::Result::eSuccess) {
        XLOGE("enumeratePhysicalDevices failed, errCode: %d", ret);
        return false;
    }
    // TODO: select suitable physicalDevice
    physicalDevice_ = physicalDevices[0];
    return true;
}

bool VkContext::QueryQueueFamilies()
{
    if (!physicalDevice_) {
        XLOGE("physicalDevice not specified!");
        return false;
    }
    uint32_t queueCnt;
    physicalDevice_.getQueueFamilyProperties(&queueCnt, nullptr);
    std::vector<vk::QueueFamilyProperties> queueProps(queueCnt);
    physicalDevice_.getQueueFamilyProperties(&queueCnt, queueProps.data());
    for (uint32_t i = 0; i < queueCnt; i++) {
        if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsQueueFamilyIdx_ = i;
        }
        if (queueProps[i].queueFlags & vk::QueueFlagBits::eCompute) {
            computeQueueFamilyIdx_ = i;
        }
        if (!(~graphicsQueueFamilyIdx_) && !(~computeQueueFamilyIdx_)) {
            break;
        }
    }
    if (!(~graphicsQueueFamilyIdx_)) {
        XLOGE("No Graphics Queue Family found!");
        return false;
    }
    if (!(~computeQueueFamilyIdx_)) {
        XLOGE("No Compute Queue Family found!");
        return false;
    }
    return true;
}

uint32_t VkContext::QueryPresentQueueFamilies(vk::SurfaceKHR surface)
{
    if (!physicalDevice_) {
        XLOGE("physicalDevice not specified!");
        return false;
    }
    uint32_t queueCnt;
    physicalDevice_.getQueueFamilyProperties(&queueCnt, nullptr);
    std::vector<vk::QueueFamilyProperties> queueProps(queueCnt);
    physicalDevice_.getQueueFamilyProperties(&queueCnt, queueProps.data());
    for (uint32_t i = 0; i < queueCnt; i++) {
        if ((queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
            auto ret = physicalDevice_.getSurfaceSupportKHR(i, surface);
            if (ret.result == vk::Result::eSuccess && ret.value == vk::True) {
                return i;
            }
        }
    }
    return UINT32_MAX;

}

bool VkContext::CreateDeviceAndQueues()
{
    if (!physicalDevice_) {
        XLOGE("physicalDevice not specified!");
        return false;
    }
    vk::DeviceCreateInfo deviceCI {};
    if (!QueryQueueFamilies()) {
        return false;
    }

    // create queue
    float priorities[] = { 1.0f };
    std::vector<vk::DeviceQueueCreateInfo> queueCIs(2);
    queueCIs[0].setQueueCount(1).setQueuePriorities(priorities).setQueueFamilyIndex(graphicsQueueFamilyIdx_);
    queueCIs[1].setQueueCount(1).setQueuePriorities(priorities).setQueueFamilyIndex(computeQueueFamilyIdx_);
    deviceCI.setQueueCreateInfoCount(2).setQueueCreateInfos(queueCIs);

    // set device extensions
    GetSupportedDeviceExtensions();
    CollectEnabledDeviceExtensions();
    deviceCI.setEnabledExtensionCount(enabledDeviceExts_.size()).setPpEnabledExtensionNames(enabledDeviceExts_.data());

    // TODO: set device layers

    // set device features: descriptor indexing
    vk::PhysicalDeviceFeatures2 features{};
    vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexing{};
    features.setPNext(&descriptorIndexing);
    /// check indexing support
    physicalDevice_.getFeatures2(&features);

    deviceCI.setPNext(&features);

    auto [ret, device] = physicalDevice_.createDeviceUnique(deviceCI, nullptr);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateDevice failed, errCode: %d", ret);
        return false;
    }
    uniqueDevice_.swap(device);
    device_ = *uniqueDevice_;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device_);
    return true;
}

bool VkContext::InitAllocator()
{
    if (!entryFunc_ || !device_) {
        return false;
    }
#define COPY_FUNCTION(NAME) vmaFuncs.vk##NAME = VULKAN_HPP_DEFAULT_DISPATCHER.vk##NAME
#define COPY_FUNCTION_KHR(NAME) vmaFuncs.vk##NAME##KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vk##NAME
    VmaVulkanFunctions vmaFuncs;
    COPY_FUNCTION(GetInstanceProcAddr);
    COPY_FUNCTION(GetDeviceProcAddr);
    COPY_FUNCTION(GetPhysicalDeviceProperties);
    COPY_FUNCTION(GetPhysicalDeviceMemoryProperties);
    COPY_FUNCTION(AllocateMemory);
    COPY_FUNCTION(FreeMemory);
    COPY_FUNCTION(MapMemory);
    COPY_FUNCTION(UnmapMemory);
    COPY_FUNCTION(FlushMappedMemoryRanges);
    COPY_FUNCTION(InvalidateMappedMemoryRanges);
    COPY_FUNCTION(BindBufferMemory);
    COPY_FUNCTION(BindImageMemory);
    COPY_FUNCTION(GetBufferMemoryRequirements);
    COPY_FUNCTION(GetImageMemoryRequirements);
    COPY_FUNCTION(CreateBuffer);
    COPY_FUNCTION(DestroyBuffer);
    COPY_FUNCTION(CreateImage);
    COPY_FUNCTION(DestroyImage);
    COPY_FUNCTION(CmdCopyBuffer);

    COPY_FUNCTION_KHR(GetBufferMemoryRequirements2);
    COPY_FUNCTION_KHR(GetImageMemoryRequirements2);
    COPY_FUNCTION_KHR(BindBufferMemory2);
    COPY_FUNCTION_KHR(BindImageMemory2);
    COPY_FUNCTION_KHR(GetPhysicalDeviceMemoryProperties2);
#undef COPY_FUNCTION
#undef COPY_FUNCTION_KHR
    VmaAllocatorCreateInfo allocatorCI {};
    allocatorCI.flags = 0;
    allocatorCI.physicalDevice = physicalDevice_;
    allocatorCI.device = device_;
    allocatorCI.preferredLargeHeapBlockSize = 1024;     // TODO: Find a property value
    allocatorCI.pVulkanFunctions = &vmaFuncs;
    allocatorCI.instance = instance_;
    allocatorCI.vulkanApiVersion = VK_API_VERSION_1_3;

    auto ret = vmaCreateAllocator(&allocatorCI, &allocator_);
    if (ret != VK_SUCCESS) {
        XLOGE("Create VmaAllocator failed, errCode: %d", ret);
        return false;
    }
    return true;
}

bool VkContext::CreateCmdPools()
{
    vk::CommandPoolCreateInfo poolCI{};
    poolCI.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolCI.setQueueFamilyIndex(graphicsQueueFamilyIdx_);
    auto [presentRet, presentPoolUnique] = device_.createCommandPoolUnique(poolCI);
    if (presentRet != vk::Result::eSuccess) {
        XLOGE("Create Present Pool failed, errCode: %d", presentRet);
        return false;
    }
    presentPool_.swap(presentPoolUnique);

    poolCI.setQueueFamilyIndex(computeQueueFamilyIdx_);
    auto [computeRet, computePoolUnique] = device_.createCommandPoolUnique(poolCI);
    if (computeRet != vk::Result::eSuccess) {
        XLOGE("Create Compute Pool failed, errCode: %d", computeRet);
        return false;
    }
    computePool_.swap(computePoolUnique);
    return true;
}

} // namespace X::Backend