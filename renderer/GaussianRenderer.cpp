#include "GaussianRenderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"

namespace X {

const std::vector<float> GaussianRenderer::vboData_ = {
    -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f
};

GaussianRenderer::GaussianRenderer()
    : Renderer(true)
{
    vbo_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ vboData_.size() * sizeof(float), BufferType::Vertex });
    vbo_->Update(vboData_.data(), vboData_.size() * sizeof(float), 0);
    SetDescriptorSetLayouts();
    SetBlendState();
    preprocessComplete_ = Backend::VkContext::GetInstance().GetDevice().createSemaphore({}).value;
}

void GaussianRenderer::SetDescriptorSetLayouts()
{
    descriptorSetLayouts_.emplace_back(Backend::DescriptorSetLayout::Make());
    descriptorSetLayouts_.emplace_back(Backend::DescriptorSetLayout::Make());
    descriptorSetLayouts_[0]->AddDescriptorBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute, 1);
    descriptorSetLayouts_[0]->AddDescriptorBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute, 4);
    descriptorSetLayouts_[0]->Update();
    descriptorSetLayouts_[1]->AddDescriptorBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute, 1);
    descriptorSetLayouts_[1]->Update();
}

void GaussianRenderer::SetBlendState()
{
    Backend::BlendState blend;
    blend.colorOp_ = 0;	// func_add
    blend.alphaOp_ = 0;	// func_add
    blend.srcColor_ = static_cast<uint32_t>(vk::BlendFactor::eOne);
    blend.dstColor_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusSrcAlpha);
    blend.srcAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOne);
    blend.dstAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusSrcAlpha);
    blend_.emplace(blend);
}

void GaussianRenderer::CreateGraphicsPipeline(std::shared_ptr<Backend::RenderPass> renderPass)
{
    std::shared_ptr<Backend::ShaderModule> vs =
        Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule(GetShaderPath() + "/gsplat_precomputed.vert", ShaderType::Vertex);
    std::shared_ptr<Backend::ShaderModule> fs =
        Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule(GetShaderPath() + "/gsplat.frag", ShaderType::Fragment);
    Backend::GraphicsPipelineInfo info = {
        "GaussianSplatting",
        renderPass,
        descriptorSetLayouts_,
        blend_,
        vs,
        fs
    };
    pipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable().RequireGraphicsPipeline(info);
}

void GaussianRenderer::CreateComputePipeline()
{
    std::shared_ptr<Backend::ShaderModule> cs =
        Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule(GetShaderPath() + "/precompute.comp", ShaderType::Compute);
    Backend::ComputePipelineInfo info = {
        "GaussianSplatting",
        descriptorSetLayouts_,
        cs
    };
    computePipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable().RequireComputePipeline(info);
}

bool GaussianRenderer::OnInit(Backend::DisplaySurface* surface)
{
    CreateGraphicsPipeline(surface->GetCurrentSwapSurface()->GetRenderPass());
    CreateComputePipeline();
    return true;
}

void GaussianRenderer::RecordComputeCommands(Scene* scene)
{
    auto cmdBuffer = GetComputeCmdBuffer();
    vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
    cmdBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    cmdBuffer.begin(cmdBufferBeginInfo);

    // TODO: prepare buffer from scene
    computePipeline_->BindStorageBuffers(scene->ssboSplatData_, 0, 0);
    computePipeline_->BindStorageBuffers(preComputed_, 1, 0);
    computePipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 1);
    computePipeline_->BindUniformBuffers(scene->uboModels_, 0, 2);
    computePipeline_->BindUniformBuffers({ scene->uboCamera_ }, 0, 3);
    computePipeline_->BindUniformBuffers({ surface_->GetScreenSizeBuffer() }, 0, 4);
    computePipeline_->BindDescriptorSets(cmdBuffer);
    if (scene->totalPointCount_ != 0) {
        cmdBuffer.dispatch((scene->totalPointCount_ + 255) / 256, 1, 1);
    }

    cmdBuffer.end();
}

void GaussianRenderer::OnRecordGraphicsCommands(Scene* scene)
{
    auto cmdBuffer = GetPresentCmdBuffer();
    vk::DeviceSize offset = 0;
    cmdBuffer.bindVertexBuffers(0, vbo_->GetHandle(), offset);
    // TODO: prepare buffer from scene
    pipeline_->BindStorageBuffers(scene->ssboSplatData_, 0, 0);
    pipeline_->BindStorageBuffers(preComputed_, 1, 0);
    pipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 1);
    pipeline_->BindUniformBuffers(scene->uboModels_, 0, 2);
    pipeline_->BindUniformBuffers({ scene->uboCamera_ }, 0, 3);
    pipeline_->BindUniformBuffers({ surface_->GetScreenSizeBuffer() }, 0, 4);
    cmdBuffer.setViewport(0, { { 0.0f, 0.0f, static_cast<float>(surface_->GetWidth()), static_cast<float>(surface_->GetHeight()), 0.0f, 1.0f } });
    cmdBuffer.setScissor(0, { { { 0, 0 }, { surface_->GetWidth(), surface_->GetHeight() } } });
    pipeline_->BindDescriptorSets(cmdBuffer);
    cmdBuffer.draw(4, scene->totalPointCount_, 0, 0);
}

void GaussianRenderer::SubmitGraphicsCommands()
{
    auto cmdBuffer = GetPresentCmdBuffer();
    auto queue = Backend::VkContext::GetInstance().AcquireGraphicsQueue(surface_->GetPresentQueueIdx());
    std::array<vk::Semaphore, 2> signalSemaphores { surface_->GetPresentWaitSemaphore(), preprocessComplete_ };
    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setCommandBuffers(cmdBuffer)
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&preprocessComplete_)
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphoreCount(2)
        .setPSignalSemaphores(signalSemaphores.data());
    auto ret = queue.submit(submitInfo);
    if (ret != vk::Result::eSuccess) {
        XLOGE("Submit graphics commands failed, errCode: %d", ret);
    }
}

void GaussianRenderer::SubmitComputeCommands()
{
    auto cmdBuffer = GetComputeCmdBuffer();

    auto queue = Backend::VkContext::GetInstance().AcquireCurrentComputeQueue().second;
    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eComputeShader };
    submitInfo.setCommandBuffers(cmdBuffer)
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&preprocessComplete_)	// wait semaphore of last frame, right?
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&preprocessComplete_);
    auto ret = queue.submit(submitInfo);
    if (ret != vk::Result::eSuccess) {
        XLOGE("Submit compute commands failed, errCode: %d", ret);
    }
}

void GaussianRenderer::OnDrawFrame()
{
    this->SubmitComputeCommands();
    this->SubmitGraphicsCommands();
}

} // namespace X
