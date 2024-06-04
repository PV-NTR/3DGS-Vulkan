#include "GraphicsPipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

GraphicsPipeline::GraphicsPipeline(std::shared_ptr<RenderPass> renderPass, const GraphicsPipelineInfo& info) noexcept
    : Pipeline(Type::Graphics)
{
    if (!CreatePipelineLayout(info)) {
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

    vk::GraphicsPipelineCreateInfo pipelineCI {};
    pipelineCI.setRenderPass(renderPass->GetHandle())
        .setPInputAssemblyState(&defaultState_.inputAssemblyState)
        .setPRasterizationState(&defaultState_.rasterizationState)
        .setPColorBlendState(&defaultState_.colorBlendState)
        .setPDepthStencilState(&defaultState_.depthStencilState)
        .setPMultisampleState(&defaultState_.multisampleState)
        .setPViewportState(&defaultState_.viewportState)
        .setPDynamicState(&defaultState_.dynamicState)
        .setStages(defaultState_.shaderStages);

    auto [ret, pipelineUnique] = VkContext::GetInstance().GetDevice().createGraphicsPipelineUnique(cache_, pipelineCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateGraphicsPipeline failed, errCode: %d", ret);
        return;
    }
    pipelineUnique_.swap(pipelineUnique);
    pipeline_ = *pipelineUnique_;
}

bool GraphicsPipeline::CreatePipelineLayout(const GraphicsPipelineInfo& info)
{
    vk::PipelineLayoutCreateInfo pipelayoutCI {};
    std::vector<vk::DescriptorSetLayout> setLayouts;
    for (const auto& layout : info.setLayouts) {
        setLayouts.emplace_back(*layout);
    }
    pipelayoutCI.setSetLayouts(setLayouts);
    auto [ret, layoutUnique] = VkContext::GetInstance().GetDevice().createPipelineLayoutUnique(pipelayoutCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreatePipelineLayout failed, errCode: %d", ret);
        return false;
    }
    layoutUnique_.swap(layoutUnique);
    return true;
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
