#pragma once

#include <memory>
#include <vector>
#include <atomic>

#include "common/vk_common.hpp"

namespace X::Backend {

class VkContext {
public:
    VkContext() = default;
    void Init();
    bool IsReady();
    [[nodiscard]] vk::Queue AcquireGraphicsQueue();
    [[nodiscard]] vk::Queue AcquireComputeQueue();

private:
    void LoadVkLibrary();
    bool CreateInstance();
    bool SelectPhysicalDevice();
    bool QueueQueueFamilies();
    bool CreateDeviceAndQueues();
    bool InitAllocator();

    void GetSupportedInstanceExtensions();
    void CollectEnabledInstanceExtensions();
    void GetSupportedInstanceLayers();
    void CollectEnabledInstanceLayers();
    void GetSupportedDeviceExtensions();
    void CollectEnabledDeviceExtensions();

private:
    vk::PhysicalDevice physicalDevice_;

    vk::UniqueInstance uniqueInstance_;
    vk::UniqueDevice uniqueDevice_;
    
    vk::Instance instance_;
    vk::Device device_;

    VmaAllocator allocator_ = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIdx_ = UINT32_MAX, computeQueueFamilyIdx_ = UINT32_MAX;
    std::atomic<uint32_t> graphicsQueueIdx_ = { 0 };
    std::atomic<uint32_t> computeQueueIdx_ = { 0 };

    PFN_vkGetInstanceProcAddr entryFunc_ = nullptr;

    static std::vector<const char*> requiredInstanceExts_, requiredInstanceLayers_;
    static std::vector<const char*> requiredDeviceExts_;

    std::vector<const char*> supportedInstanceExts_, supportedInstanceLayers_;
    std::vector<const char*> supportedDeviceExts_;

    std::vector<const char*> enabledInstanceExts_, enabledInstanceLayers_;
    std::vector<const char*> enabledDeviceExts_;
};

} // namespace X::Backend