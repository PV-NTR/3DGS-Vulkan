#include "GraphicsPipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info, vk::PipelineCache cache) noexcept
    : Pipeline(info.name + "-graphics", Type::Graphics)
{
    assert(!info.setLayouts.empty());
    if (!CreatePipelineLayout(info.setLayouts)) {
        return;
    }

    InitDefaultSettings();

    vk::PipelineColorBlendAttachmentState blendAttachmentState {};
    blendAttachmentState.setBlendEnable(info.blend.has_value() ? vk::True : vk::False);
    if (info.blend.has_value()) {
        auto& blendState = info.blend.value();
        blendAttachmentState.setSrcColorBlendFactor(static_cast<vk::BlendFactor>(blendState.srcColor_))
            .setDstColorBlendFactor(static_cast<vk::BlendFactor>(blendState.dstColor_))
            .setSrcAlphaBlendFactor(static_cast<vk::BlendFactor>(blendState.srcAlpha_))
            .setDstAlphaBlendFactor(static_cast<vk::BlendFactor>(blendState.dstAlpha_))
            .setColorBlendOp(blendState.GetColorBlendOp())
            .setAlphaBlendOp(blendState.GetAlphaBlendOp());
    }
    defaultState_.colorBlendState.setAttachments(blendAttachmentState);

    assert(info.vs->GetType() == ShaderType::Vertex);
    assert(info.fs->GetType() == ShaderType::Fragment);
    defaultState_.shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex).setModule(info.vs->GetHandle());
    defaultState_.shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment).setModule(info.fs->GetHandle());

    assert(info.renderPass != nullptr);
    vk::GraphicsPipelineCreateInfo pipelineCI {};
    pipelineCI.setRenderPass(info.renderPass->GetHandle())
        .setLayout(*layoutUnique_)
        .setPInputAssemblyState(&defaultState_.inputAssemblyState)
        .setPRasterizationState(&defaultState_.rasterizationState)
        .setPColorBlendState(&defaultState_.colorBlendState)
        .setPDepthStencilState(&defaultState_.depthStencilState)
        .setPMultisampleState(&defaultState_.multisampleState)
        .setPViewportState(&defaultState_.viewportState)
        .setPDynamicState(&defaultState_.dynamicState)
        .setStages(defaultState_.shaderStages);

    auto [ret, pipelineUnique] = VkContext::GetInstance().GetDevice().createGraphicsPipelineUnique(cache, pipelineCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateGraphicsPipeline failed, errCode: %d", ret);
        return;
    }
    pipelineUnique_.swap(pipelineUnique);
    pipeline_ = *pipelineUnique_;
    this->DependOn(info.vs);
    this->DependOn(info.fs);
    for (const auto& setLayout : info.setLayouts) {
        this->DependOn(setLayout);
    }
}

void GraphicsPipeline::InitDefaultSettings()
{
    defaultState_.inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(vk::False);
    defaultState_.rasterizationState.setPolygonMode(vk::PolygonMode::eFill).setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eClockwise);
    defaultState_.multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    defaultState_.depthStencilState.setDepthTestEnable(vk::True).setStencilTestEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual);
    defaultState_.viewportState.setViewportCount(1).setScissorCount(1);
    std::array<vk::DynamicState, 2> dynamicStateEnables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    defaultState_.dynamicState.setDynamicStates(dynamicStateEnables);
}

} // namespace X::Backend
