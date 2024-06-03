#include "GraphicsPipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

GraphicsPipeline::GraphicsPipeline(std::shared_ptr<RenderPass> renderPass, const GraphicsPipelineInfo& info) noexcept
	: Pipeline(Type::Graphics)
{
	vk::GraphicsPipelineCreateInfo pipelineCI {};
	pipelineCI.setRenderPass(renderPass->GetHandle());


	auto [ret, pipelineUnique] = VkContext::GetInstance().GetDevice().createGraphicsPipelineUnique(cache_, pipelineCI);
	if (ret != vk::Result::eSuccess) {
		XLOGE("CreateGraphicsPipeline failed, errCode: %d", ret);
		return;
	}
	pipelineUnique_.swap(pipelineUnique);
	pipeline_ = *pipelineUnique_;
}

} // namespace X::Backend
