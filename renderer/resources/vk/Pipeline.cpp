#include "Pipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

Pipeline::~Pipeline()
{

}

Pipeline::Pipeline(std::string name, const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts, Type type) noexcept
    : name_(name), type_(type)
{
    assert(!setLayouts.empty());
    CreateDescriptorPool(setLayouts);
    CreatePipelineLayout(setLayouts);
    AllocDescriptorSets(setLayouts);
}

bool Pipeline::CreateDescriptorPool(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts)
{
    vk::DescriptorPoolCreateInfo poolCI {};
    std::vector<vk::DescriptorPoolSize> poolSizes;
    for (const auto& layout : setLayouts) {
        poolSizes.emplace_back();
        auto& bindingInfo = layout->bindingInfo_;
        for (const auto& binding : bindingInfo) {
            if (binding.second == 0) {
                continue;
            }
            poolSizes.back().setType(DescriptorSetLayout::descTypeMap_[binding.first.type]);
        }
    }
    poolCI.setPoolSizes(poolSizes);

    auto [ret, poolUnique] = VkContext::GetInstance().GetDevice().createDescriptorPoolUnique(poolCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateDescriptorPool failed, errCode: %d", ret);
        return false;
    }
    pool_.swap(poolUnique);
    return true;
}

bool Pipeline::CreatePipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts)
{
    vk::PipelineLayoutCreateInfo pipelayoutCI{};
    std::vector<vk::DescriptorSetLayout> setLayoutsData;
    for (const auto& layout : setLayouts) {
        setLayoutsData.emplace_back(layout->GetHandle());
    }
    pipelayoutCI.setSetLayouts(setLayoutsData);
    auto [ret, layoutUnique] = VkContext::GetInstance().GetDevice().createPipelineLayoutUnique(pipelayoutCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreatePipelineLayout failed, errCode: %d", ret);
        return false;
    }
    layout_ .swap(layoutUnique);
    return true;
}

bool Pipeline::AllocDescriptorSets(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts)
{
    vk::DescriptorSetAllocateInfo allocInfo{};
    std::vector<vk::DescriptorSetLayout> setLayoutsData;
    for (const auto& layout : setLayouts) {
        setLayoutsData.emplace_back(layout->GetHandle());
    }
    allocInfo.setDescriptorPool(*pool_).setSetLayouts(setLayoutsData);
    auto [ret, descriptorSetsUnique] = VkContext::GetInstance().GetDevice().allocateDescriptorSetsUnique(allocInfo);
    if (ret != vk::Result::eSuccess) {
        XLOGE("allocateDescriptorSets failed, errCode: %d", ret);
        return false;
    }

    for (auto&& descriptorSetUnique : descriptorSetsUnique) {
        descriptorSets_.emplace_back(std::move(descriptorSetUnique));
    }
    return true;
}

bool Pipeline::BindUniformBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet, uint32_t binding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }
    
    std::vector<vk::DescriptorBufferInfo> infos;
    for (const auto& buffer : buffers) {
        if (buffer->GetType() != BufferType::Uniform) {
            XLOGE("input buffer not uniform buffer!");
            return false;
        }
        infos.emplace_back(buffer->GetHandle(), 0, buffer->GetSize());
    }
    vk::WriteDescriptorSet dWrite{};
    dWrite.setDstSet(*descriptorSets_[bindSet])
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDstBinding(binding)
        .setBufferInfo(infos);
    VkContext::GetInstance().GetDevice().updateDescriptorSets(dWrite, nullptr);
    return true;
}

bool Pipeline::BindStorageBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet, uint32_t binding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }
    std::vector<vk::DescriptorBufferInfo> infos;
    for (const auto& buffer : buffers) {
        if (buffer->GetType() != BufferType::Storage) {
            XLOGE("input buffer not storage buffer!");
            return false;
        }
        infos.emplace_back(buffer->GetHandle(), 0, buffer->GetSize());
    }
    vk::WriteDescriptorSet dWrite{};
    dWrite.setDstSet(*descriptorSets_[bindSet])
        .setDescriptorType(vk::DescriptorType::eStorageBuffer)
        .setDstBinding(binding)
        .setBufferInfo(infos);
    VkContext::GetInstance().GetDevice().updateDescriptorSets(dWrite, nullptr);
    return true;
}

bool Pipeline::BindTextures(const std::vector<std::shared_ptr<Image>>& images, uint32_t bindSet, uint32_t binding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }
    XLOGE("NOT IMPLEMENT YET!");
    return false;
}

} // namespace X::Backend
