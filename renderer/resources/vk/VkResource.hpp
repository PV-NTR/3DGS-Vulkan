#pragma once

#include <common/VkCommon.hpp>
#include "VkResourceBase.hpp"

namespace X::Backend {

template <typename VkUniqueHandle>
class VkResource : public VkUniqueHandle, public VkResourceBase {
public:
    VkResource() = default;

    VkResource(VkUniqueHandle&& uniqueHandle) : VkUniqueHandle(std::move(uniqueHandle)) {}
    VkResource(VkResource&& other) noexcept : VkUniqueHandle(other.release())
    {
        this->dependencies_ = std::move(other.dependencies_);
    }
    VkResource& operator=(VkResource&& other) = delete;
    virtual ~VkResource() { VkUniqueHandle::reset(); }
};

using PipelineLayout = VkResource<vk::UniquePipelineLayout>;
using DescriptorPool = VkResource<vk::UniqueDescriptorPool>;
using DescriptorSet = VkResource<vk::UniqueDescriptorSet>;
using CommandPool = VkResource<vk::UniqueCommandPool>;
using Swapchain = VkResource<vk::UniqueSwapchainKHR>;
using Framebuffer = VkResource<vk::UniqueFramebuffer>;

} // namespace X::Backend
