#pragma once

#include <common/VkCommon.hpp>
#include "VkResourceBase.hpp"

namespace X::Backend {

template <typename VkUniqueHandle>
class VkResource : public VkUniqueHandle, public VkResourceBase {
public:
    virtual ~VkResource() { VkUniqueHandle::reset(); }
};

using DescriptorSetLayout = VkResource<vk::UniqueDescriptorSetLayout>;

} // namespace X::Backend
