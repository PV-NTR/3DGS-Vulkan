#pragma once

#include "common/VkCommon.hpp"
#include "VkResourceBase.hpp"
#include "resources/Enums.hpp"

namespace X::Backend {

struct BindingTypes {
    vk::DescriptorType type_;
    vk::ShaderStageFlags stage_;
};

using LayoutBindingInfo = std::pair<BindingTypes, uint32_t>;

class DescriptorSetLayout : public VkResourceBase {
public:
    [[nodiscard]] static std::shared_ptr<DescriptorSetLayout> Make();
    virtual ~DescriptorSetLayout() noexcept;
    vk::DescriptorSetLayout GetHandle() { return descLayout_; }

    void Update();
    void AddDescriptorBinding(vk::DescriptorType type_, vk::ShaderStageFlags stage_, uint32_t num);

protected:
    DescriptorSetLayout() noexcept = default;

private:
    friend class Pipeline;
    std::vector<LayoutBindingInfo> bindingInfos_;
    vk::UniqueDescriptorSetLayout uniqueDescLayout_;
    vk::DescriptorSetLayout descLayout_;
};
    
} // namespace X::Backend
