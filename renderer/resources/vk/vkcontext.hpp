#pragma once

#include <memory>

#include "common/vk_common.hpp"

namespace X::Backend {

class VkContext {
public:
    VkContext() = default;
    void Init();
    bool IsReady();

private:
    void LoadLibrary();
    void CreateInstance();
    void SelectPhysicalDevice();
    void CreateDeviceAndQueues();
    void InitAllocator();

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
};

} // namespace X::Backend