#pragma once

#include "common/VkCommon.hpp"
#include "VkResourceBase.hpp"
#include "resources/Enums.hpp"

namespace X::Backend {

union BindingTypes {
    struct {
        uint32_t type : 4;
        uint32_t stage : 14;
        uint32_t padding : 14;
    };
    uint32_t packed;
};

using LayoutBindingInfo = std::array<std::pair<BindingTypes, uint32_t>, 4>;

class DescriptorSetLayout : public VkResourceBase {
public:
    explicit DescriptorSetLayout(const LayoutBindingInfo& bindingInfo);
    virtual ~DescriptorSetLayout();
    vk::DescriptorSetLayout GetHandle() { return descLayout_; }

private:
    friend class Pipeline;
    static const std::array<vk::DescriptorType, 4> descTypeMap_;
    static const std::array<vk::ShaderStageFlagBits, 14> shaderStageMap_;
    LayoutBindingInfo bindingInfo_;
    vk::UniqueDescriptorSetLayout uniqueDescLayout_;
    vk::DescriptorSetLayout descLayout_;
};
    
} // namespace X::Backend
