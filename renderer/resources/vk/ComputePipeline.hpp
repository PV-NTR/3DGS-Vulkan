#pragma once

#include "Pipeline.hpp"

namespace X::Backend {

struct ComputePipelineInfo {

};

class ComputePipeline : public Pipeline {
public:
	explicit ComputePipeline(const ComputePipelineInfo& info) noexcept;
	virtual ~ComputePipeline() = default;

private:
	std::vector<vk::ShaderModule> shaders_;
};

} // namespace X::Backend