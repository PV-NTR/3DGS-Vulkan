#pragma once

#include <optional>
#include "Pipeline.hpp"

namespace X::Backend {

struct GraphicsPipelineInfo {

};

class GraphicsPipeline : public Pipeline {
public:
	explicit GraphicsPipeline(std::shared_ptr<RenderPass> renderPass, const GraphicsPipelineInfo& info) noexcept;
	virtual ~GraphicsPipeline() = default;

private:
	vk::ShaderModule vs_;
	vk::ShaderModule fs_;
	std::optional<vk::ShaderModule> tes_, tcs_;
	std::optional<vk::ShaderModule> gs_;
};

} // namespace X::Backend