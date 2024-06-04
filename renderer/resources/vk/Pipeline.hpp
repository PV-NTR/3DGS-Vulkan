#pragma once

#include "common/VkCommon.hpp"
#include "RenderPass.hpp"
#include "VkResourceBase.hpp"

namespace X::Backend {

class Pipeline : public VkResourceBase {
public:
    enum class Type {
        Graphics = 0,
        Compute,
    };
    virtual ~Pipeline();

protected:
    explicit Pipeline(Type type = Type::Graphics) noexcept : type_(type) {}

    Type type_;
    vk::UniquePipeline pipelineUnique_;
    vk::Pipeline pipeline_;
    vk::PipelineCache cache_;
    vk::UniquePipelineLayout layoutUnique_;
};

} // namespace X::Backend