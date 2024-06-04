#pragma once

#include <optional>
#include "Pipeline.hpp"
#include "VkResource.hpp"
#include "BlendState.hpp"
#include "ShaderModule.hpp"

namespace X::Backend {

struct GraphicsPipelineInfo {
	std::vector<DescriptorSetLayout> setLayouts;
	std::optional<BlendState> blend;
	std::shared_ptr<ShaderModule> vs;
	std::shared_ptr<ShaderModule> fs;
};

class GraphicsPipeline : public Pipeline {
public:
	explicit GraphicsPipeline(std::shared_ptr<RenderPass> renderPass, const GraphicsPipelineInfo& info) noexcept;
	virtual ~GraphicsPipeline() = default;

protected:
	bool CreatePipelineLayout(const GraphicsPipelineInfo& info);
	void InitDefaultSettings();
	struct {
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		vk::PipelineMultisampleStateCreateInfo multisampleState;
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::PipelineDynamicStateCreateInfo dynamicState;
		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
	} defaultState_;

private:
	vk::ShaderModule vs_;
	vk::ShaderModule fs_;
	std::optional<vk::ShaderModule> tes_, tcs_;
	std::optional<vk::ShaderModule> gs_;
};

} // namespace X::Backend