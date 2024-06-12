#pragma once

#include <memory>
#include <vector>
#include <atomic>

#include "common/VkCommon.hpp"
#include "utils/Singleton.hpp"

namespace X::Backend {

class VkContext : public Singleton<VkContext>{
public:
    VkContext() = default;
    void Init();
    bool IsReady();
    [[nodiscard]] vk::Queue AcquireGraphicsQueue(uint32_t idx);
    [[nodiscard]] vk::Queue AcquireComputeQueue(uint32_t idx);
    [[nodiscard]] std::pair<uint32_t, vk::Queue> AcquireCurrentGraphicsQueue();
    [[nodiscard]] std::pair<uint32_t, vk::Queue> AcquireCurrentComputeQueue();
    [[nodiscard]] std::pair<uint32_t, vk::Queue> AcquireNextGraphicsQueue();
    [[nodiscard]] std::pair<uint32_t, vk::Queue> AcquireNextComputeQueue();

    VmaAllocator GetAllocator()
    {
        return allocator_;
    }

    vk::Device GetDevice()
    {
        return device_;
    }

private:
    friend class DisplaySurface;
    void LoadVkLibrary();
    bool CreateInstance();
    bool SelectPhysicalDevice();
    bool QueryQueueFamilies();
    bool CreateDeviceAndQueues();
    bool InitAllocator();

    void GetSupportedInstanceExtensions();
    void CollectEnabledInstanceExtensions();
    void GetSupportedInstanceLayers();
    void CollectEnabledInstanceLayers();
    void GetSupportedDeviceExtensions();
    void CollectEnabledDeviceExtensions();

    uint32_t QueryPresentQueueFamilies(vk::SurfaceKHR surface);

    vk::PhysicalDevice GetPhysicalDevice()
    {
        return physicalDevice_;
    }

    vk::Instance GetVkInstance()
    {
        return instance_;
    }


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

    std::vector<std::string> supportedInstanceExts_, supportedInstanceLayers_;
    std::vector<std::string> supportedDeviceExts_;

    std::vector<const char*> enabledInstanceExts_, enabledInstanceLayers_;
    std::vector<const char*> enabledDeviceExts_;
};

} // namespace X::Backend