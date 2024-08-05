#include "DescriptorSetLayout.hpp"

#include "common/LogCommon.hpp"
#include "VkContext.hpp"

namespace X::Backend {

std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::Make()
{
    return std::shared_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
}

void DescriptorSetLayout::Update()
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const auto& bindingInfo : bindingInfos_) {
        for (uint32_t i = 0; i < bindingInfo.second; i++) {
            bindings.emplace_back();
            bindings.back().setDescriptorType(bindingInfo.first.type_)
                .setDescriptorCount(1).setStageFlags(bindingInfo.first.stage_).setBinding(bindings.size() - 1);
        }
    }
    vk::DescriptorSetLayoutCreateInfo layoutCI{};
    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCI{};
    std::vector<vk::DescriptorBindingFlags> bindingFlags(bindings.size(),
        vk::DescriptorBindingFlagBits::eUpdateAfterBind);
    bindingFlagsCI.setBindingFlags(bindingFlags);
    layoutCI.setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool).setBindings(bindings)
        .setPNext(&bindingFlagsCI);
    auto [ret, uniqueDescLayout] = VkContext::GetInstance().GetDevice().createDescriptorSetLayoutUnique(layoutCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateDescriptorSetLayout failed, errCode: %d", ret);
        return;
    }
    uniqueDescLayout_.swap(uniqueDescLayout);
    descLayout_ = *uniqueDescLayout_;
}

void DescriptorSetLayout::AddDescriptorBinding(vk::DescriptorType type, vk::ShaderStageFlags stage, uint32_t num)
{
    bindingInfos_.emplace_back(BindingTypes{ type, stage }, num);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

} // namespace X::Backend
