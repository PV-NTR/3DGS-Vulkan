#include "ComputePipeline.hpp"

#include "common/LogCommon.hpp"
#include "VkContext.hpp"

namespace X::Backend {

ComputePipeline::ComputePipeline(const ComputePipelineInfo& info, vk::PipelineCache cache) noexcept
    : Pipeline(info.name + "-compute", Type::Compute)
{
    assert(!info.setLayouts.empty());
    if (!CreatePipelineLayout(info.setLayouts)) {
        return;
    }

    assert(info.cs->GetType() == vk::ShaderStageFlagBits::eCompute);
    vk::PipelineShaderStageCreateInfo shaderStage {};
    shaderStage.setStage(vk::ShaderStageFlagBits::eCompute).setModule(info.cs->GetHandle());

    vk::ComputePipelineCreateInfo pipelineCI {};
    pipelineCI.setLayout(*layoutUnique_).setStage(shaderStage);

    auto [ret, pipelineUnique] = VkContext::GetInstance().GetDevice().createComputePipelineUnique(cache, pipelineCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateComputePipeline failed, errCode: %d", ret);
        return;
    }
    pipelineUnique_.swap(pipelineUnique);
    pipeline_ = *pipelineUnique_;
}

} // namespace X::Backend
