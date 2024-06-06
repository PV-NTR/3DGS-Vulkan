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
    virtual ~Pipeline();

    bool BindUniformBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet, uint32_t binding);
    bool BindStorageBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers, uint32_t bindSet, uint32_t binding);
    bool BindTextures(const std::vector<std::shared_ptr<Image>>& images, uint32_t bindSet, uint32_t binding);

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
    vk::UniquePipelineLayout layoutUnique_;
    vk::PipelineLayout layout_;
    vk::UniqueDescriptorPool poolUnique_;
    vk::DescriptorPool pool_;
    std::vector<vk::UniqueDescriptorSet> descriptorSetsUnique_;
    std::vector<vk::DescriptorSet> descriptorSets_;
};

} // namespace X::Backend