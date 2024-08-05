#include "Pipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

Pipeline::~Pipeline()
{
    descriptorSets_.clear();
}

Pipeline::Pipeline(std::string name, const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts,
    Type type) noexcept : name_(name), type_(type)
{
    assert(!setLayouts.empty());
    CreateDescriptorPool(setLayouts);
    CreatePipelineLayout(setLayouts);
    AllocDescriptorSets(setLayouts);
}

bool Pipeline::CreateDescriptorPool(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts)
{
    vk::DescriptorPoolCreateInfo poolCI {};
    std::unordered_map<vk::DescriptorType, uint32_t> descriptorCnts;
    uint32_t setCnt = 0;
    for (const auto& layout : setLayouts) {
        auto& bindingInfos = layout->bindingInfos_;
        for (const auto& bindingInfo : bindingInfos) {
            if (bindingInfo.second == 0) {
                continue;
            }
            setCnt += bindingInfo.second;
            descriptorCnts[bindingInfo.first.type_] += bindingInfo.second;
        }
    }
    std::vector<vk::DescriptorPoolSize> poolSizes;
    for (auto&& [key, value] : descriptorCnts) {
        poolSizes.emplace_back();
        poolSizes.back().setType(key).setDescriptorCount(value);
    }
    poolCI.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind).setPoolSizes(poolSizes).setMaxSets(setCnt);

    auto [ret, poolUnique] = VkContext::GetInstance().GetDevice().createDescriptorPoolUnique(poolCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateDescriptorPool failed, errCode: %d", ret);
        return false;
    }
    pool_ = std::make_shared<DescriptorPool>(std::move(poolUnique));
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
    allocInfo.setDescriptorPool(pool_->get()).setSetLayouts(setLayoutsData);
    auto [ret, descriptorSetsUnique] = VkContext::GetInstance().GetDevice().allocateDescriptorSetsUnique(allocInfo);
    if (ret != vk::Result::eSuccess) {
        XLOGE("allocateDescriptorSets failed, errCode: %d", ret);
        return false;
    }

    for (auto&& descriptorSetUnique : descriptorSetsUnique) {
        descriptorSets_.emplace_back(std::move(descriptorSetUnique));
        descriptorSets_.back().DependOn(pool_);
    }
    return true;
}

bool Pipeline::BindBuffer(std::shared_ptr<Buffer> buffer, uint32_t bindSet, uint32_t binding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }
    vk::DescriptorBufferInfo info {};
    info.setBuffer(buffer->GetHandle()).setOffset(0).setRange(buffer->GetSize());

    vk::WriteDescriptorSet dWrite {};
    dWrite.setDstSet(*descriptorSets_[bindSet])
        .setDescriptorCount(1)
        .setDescriptorType(buffer->GetType() == BufferType::Uniform ?
            vk::DescriptorType::eUniformBuffer : vk::DescriptorType::eStorageBuffer)
        .setDstBinding(binding)
        .setBufferInfo(info);
    VkContext::GetInstance().GetDevice().updateDescriptorSets(dWrite, nullptr);
    return true;
}

bool Pipeline::BindUniformBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet,
    uint32_t startBinding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }
    if (buffers.empty()) {
        XLOGE("No buffer to bind!");
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
    std::vector<vk::WriteDescriptorSet> dWrites(buffers.size());
    for (uint32_t i = 0; i < buffers.size(); i++) {
        dWrites[i].setDstSet(*descriptorSets_[bindSet])
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDstBinding(startBinding + i)
            .setBufferInfo(infos[i]);
    }
    VkContext::GetInstance().GetDevice().updateDescriptorSets(dWrites, nullptr);
    return true;
}

bool Pipeline::BindStorageBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet,
    uint32_t startBinding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }
    if (buffers.empty()) {
        XLOGE("No buffer to bind!");
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
    std::vector<vk::WriteDescriptorSet> dWrites(buffers.size());
    for (uint32_t i = 0; i < buffers.size(); i++) {
        dWrites[i].setDstSet(*descriptorSets_[bindSet])
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDstBinding(startBinding + i)
            .setBufferInfo(infos[i]);
    }
    VkContext::GetInstance().GetDevice().updateDescriptorSets(dWrites, nullptr);
    return true;
}

bool Pipeline::BindTextures(const std::vector<std::pair<std::shared_ptr<Image>, std::shared_ptr<Sampler>>>& images,
    uint32_t bindSet, uint32_t startBinding)
{
    if (bindSet >= descriptorSets_.size()) {
        XLOGE("Bind set index not valid!");
        return false;
    }

    if (images.empty()) {
        XLOGE("No buffer to bind!");
        return false;
    }

    std::vector<vk::DescriptorImageInfo> infos;
    for (const auto& [image, sampler] : images) {
        infos.emplace_back(sampler->get(), image->GetView()->GetHandle(), vk::ImageLayout::eShaderReadOnlyOptimal);
    }
    std::vector<vk::WriteDescriptorSet> dWrites(images.size());
    for (uint32_t i = 0; i < images.size(); i++) {
        dWrites[i].setDstSet(*descriptorSets_[bindSet])
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDstBinding(startBinding + i)
            .setImageInfo(infos[i]);
    }
    VkContext::GetInstance().GetDevice().updateDescriptorSets(dWrites, nullptr);
    return true;
}

bool Pipeline::BindDescriptorSets(std::shared_ptr<CommandBuffer> commandBuffer)
{
    auto cmdBuffer = commandBuffer->get();
    vk::PipelineBindPoint bindPoint =
        type_ == Type::Graphics ? vk::PipelineBindPoint::eGraphics : vk::PipelineBindPoint::eCompute;
    cmdBuffer.bindPipeline(bindPoint, pipeline_);
    std::vector<vk::DescriptorSet> descSetHandles;
    for (const auto& descriptorSet : descriptorSets_) {
        descSetHandles.emplace_back(descriptorSet.get());
    }
    cmdBuffer.bindDescriptorSets(bindPoint, *layout_, 0, descSetHandles, {});
    return true;
}

} // namespace X::Backend
