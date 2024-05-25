#pragma once

#include <memory>
#include <vector>

#include "common/vk_common.hpp"

namespace X::Backend {

class VkContext {
public:
    VkContext() = default;
    void Init();
    bool IsReady();

private:
    void LoadVkLibrary();
    void CreateInstance();
    void SelectPhysicalDevice();
    void CreateDeviceAndQueues();
    void InitAllocator();

    void GetSupportedInstanceExtensions();
    void CollectEnabledInstanceExtensions();

private:
    vk::PhysicalDevice physicalDevice_;

    vk::UniqueInstance uniqueInstance_;
    vk::UniqueDevice uniqueDevice_;
    
    vk::Instance instance_;
    vk::Device device_;

    VmaAllocator allocator_ = VK_NULL_HANDLE;
    struct {
        vk::Queue graphic {};
        vk::Queue compute {};
        // vk::Queue transfer {};
    } queues_;

    PFN_vkGetInstanceProcAddr entryFunc_ = nullptr;

    static std::vector<const char*> requiredInstanceExts, requiredInstanceLayers;
    static std::vector<const char*> requiredDeviceExts;

    std::vector<const char*> supportedInstanceExts, supportedInstanceLayers;
    std::vector<const char*> supportedDeviceExts;

    std::vector<const char*> enabledInstanceExts, enabledInstanceLayers;
    std::vector<const char*> enabledDeviceExts;
};

} // namespace X::Backend