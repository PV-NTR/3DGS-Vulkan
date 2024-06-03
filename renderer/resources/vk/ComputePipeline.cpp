#include "ComputePipeline.hpp"

#include "common/LogCommon.hpp"
#include "VkContext.hpp"

namespace X::Backend {

ComputePipeline::ComputePipeline(const ComputePipelineInfo& info) noexcept
	: Pipeline(Type::Compute)
{
	vk::ComputePipelineCreateInfo pipelineCI{};

	auto [ret, pipelineUnique] = VkContext::GetInstance().GetDevice().createComputePipelineUnique(cache_, pipelineCI);
	if (ret != vk::Result::eSuccess) {
		XLOGE("CreateGraphicsPipeline failed, errCode: %d", ret);
		return;
	}
	pipelineUnique_.swap(pipelineUnique);
	pipeline_ = *pipelineUnique_;
}

} // namespace X::Backend
