#include "GraphicsPipeline.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info, vk::PipelineCache cache) noexcept
    : Pipeline(info.name + "-graphics", info.setLayouts, Type::Graphics)
{
    InitDefaultSettings();

    vk::PipelineColorBlendAttachmentState blendAttachmentState {};
    blendAttachmentState.setBlendEnable(info.blend.has_value() ? vk::True : vk::False)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
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
    defaultState_.shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex).setModule(info.vs->GetHandle()).setPName("main");
    defaultState_.shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment).setModule(info.fs->GetHandle()).setPName("main");

    assert(info.renderPass != nullptr);
    vk::GraphicsPipelineCreateInfo pipelineCI {};
    if (info.renderPass->DepthStencilAttachmentID() != UINT32_MAX) {
        defaultState_.depthStencilState.setDepthTestEnable(vk::True).setStencilTestEnable(vk::False).setDepthCompareOp(vk::CompareOp::eLessOrEqual);
    }
    pipelineCI.setRenderPass(info.renderPass->GetHandle())
        .setLayout(*layout_)
        .setPVertexInputState(&defaultState_.vertexInputState)
        .setPInputAssemblyState(&defaultState_.inputAssemblyState)
        .setPRasterizationState(&defaultState_.rasterizationState)
        .setPColorBlendState(&defaultState_.colorBlendState)
        .setPDepthStencilState(&defaultState_.depthStencilState)
        .setPMultisampleState(&defaultState_.multisampleState)
        .setPDynamicState(&defaultState_.dynamicState)
        .setPViewportState(&defaultState_.viewportState)
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
    // Specify vertex input state
    defaultState_.vertexInputBinding.setBinding(0).setStride(8).setInputRate(vk::VertexInputRate::eVertex);
    defaultState_.vertexInputAttribute.setLocation(0).setBinding(0).setFormat(vk::Format::eR32G32Sfloat);
    defaultState_.vertexInputState.setVertexBindingDescriptions(defaultState_.vertexInputBinding)
        .setVertexAttributeDescriptions(defaultState_.vertexInputAttribute);
    defaultState_.inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(vk::False);
    defaultState_.rasterizationState.setPolygonMode(vk::PolygonMode::eFill).setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eClockwise);
    defaultState_.multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    defaultState_.depthStencilState.setDepthTestEnable(vk::False).setStencilTestEnable(vk::False);
    defaultState_.viewportState.setViewportCount(1).setScissorCount(1);
    defaultState_.dynamicState.setDynamicStates(defaultState_.dynamicStateEnables);
}

} // namespace X::Backend
