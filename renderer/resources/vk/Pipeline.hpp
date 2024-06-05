#pragma once

#include "common/VkCommon.hpp"
#include "RenderPass.hpp"
#include "VkResource.hpp"

namespace X::Backend {

class Pipeline : public VkResourceBase {
public:
    enum class Type {
        Graphics = 0,
        Compute,
    };
    virtual ~Pipeline();

protected:
    Pipeline(std::string name, Type type = Type::Graphics) noexcept : name_(name), type_(type) {}
    bool CreatePipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>>& setLayouts);

    std::string name_;
    Type type_;
    vk::UniquePipeline pipelineUnique_;
    vk::Pipeline pipeline_;
    vk::UniquePipelineLayout layoutUnique_;
};

} // namespace X::Backend