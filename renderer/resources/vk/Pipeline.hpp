#pragma once

#include "common/VkCommon.hpp"
#include "RenderPass.hpp"
#include "VkResource.hpp"
#include "DescriptorSetLayout.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

namespace X::Backend {

class Pipeline : public VkResourceBase {
public:
    enum class Type {
        Graphics = 0,
        Compute,
    };
    ~Pipeline() override;

    bool BindBuffer(std::shared_ptr<Buffer> buffer, uint32_t bindSet, uint32_t binding);
    bool BindUniformBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet,
        uint32_t startBinding);
    bool BindStorageBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet,
        uint32_t startBinding);
    // TODO: sampler type
    bool BindTextures(const std::vector<std::pair<std::shared_ptr<Image>, std::shared_ptr<Sampler>>>& images,
        uint32_t bindSet, uint32_t startBinding);
    bool BindDescriptorSets(std::shared_ptr<CommandBuffer> commandBuffer);
    vk::PipelineLayout GetLayout() const
    {
        return layout_.get();
    }

protected:
    Pipeline(std::string name, const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts,
        Type type = Type::Graphics) noexcept;
    bool CreateDescriptorPool(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts);
    bool CreatePipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts);
    bool AllocDescriptorSets(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts);

    std::string name_;
    Type type_;
    vk::UniquePipeline pipelineUnique_;
    vk::Pipeline pipeline_;
    PipelineLayout layout_;
    std::shared_ptr<DescriptorPool> pool_;
    std::vector<DescriptorSet> descriptorSets_;
};

} // namespace X::Backend