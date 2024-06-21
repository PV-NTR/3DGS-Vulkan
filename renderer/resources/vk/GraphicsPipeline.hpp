#pragma once

#include <optional>
#include "Pipeline.hpp"
#include "BlendState.hpp"
#include "ShaderModule.hpp"

namespace X::Backend {

struct GraphicsPipelineInfo {
    std::string name;
    std::shared_ptr<RenderPass> renderPass;
    std::vector<std::shared_ptr<DescriptorSetLayout>> setLayouts;
    std::optional<BlendState> blend;
    std::shared_ptr<ShaderModule> vs;
    std::shared_ptr<ShaderModule> fs;
};

class GraphicsPipeline : public Pipeline {
private:
    friend class PipelineTable;

public:
    virtual ~GraphicsPipeline() = default;

protected:
    GraphicsPipeline(const GraphicsPipelineInfo& info, vk::PipelineCache cache) noexcept;

protected:
    void InitDefaultSettings();
    struct {
        vk::VertexInputBindingDescription vertexInputBinding;
        vk::VertexInputAttributeDescription vertexInputAttribute;
        vk::PipelineVertexInputStateCreateInfo vertexInputState;
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineDynamicStateCreateInfo dynamicState;
        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
        std::array<vk::DynamicState, 2> dynamicStateEnables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    } defaultState_;

private:
    std::string name_;
};

} // namespace X::Backend