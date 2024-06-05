#include "Pipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

Pipeline::~Pipeline()
{

}

bool Pipeline::CreatePipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts)
{
    vk::PipelineLayoutCreateInfo pipelayoutCI{};
    std::vector<vk::DescriptorSetLayout> setLayoutsData;
    for (const auto& layout : setLayouts) {
        setLayoutsData.emplace_back(**layout);
    }
    pipelayoutCI.setSetLayouts(setLayoutsData);
    auto [ret, layoutUnique] = VkContext::GetInstance().GetDevice().createPipelineLayoutUnique(pipelayoutCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreatePipelineLayout failed, errCode: %d", ret);
        return false;
    }
    layoutUnique_.swap(layoutUnique);
    return true;
}

} // namespace X::Backend
