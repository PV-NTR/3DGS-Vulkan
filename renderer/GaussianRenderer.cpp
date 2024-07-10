#include "GaussianRenderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"

namespace X {

const std::vector<float> GaussianRenderer::vboData_ = {
    -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f
};

const std::vector<uint16_t> GaussianRenderer::iboData_ = {
    0, 1, 2, 2, 3, 0
};

GaussianRenderer::GaussianRenderer()
    : Renderer(true)
{
    vbo_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ vboData_.size() * sizeof(float), BufferType::Vertex });
    vbo_->Update(vboData_.data(), vboData_.size() * sizeof(float), 0);
    ibo_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ iboData_.size() * sizeof(uint16_t), BufferType::Index });
    ibo_->Update(iboData_.data(), iboData_.size() * sizeof(uint16_t), 0);
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
    descriptorSetLayouts_[1]->AddDescriptorBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex, 1);
    descriptorSetLayouts_[1]->Update();
}

void GaussianRenderer::SetBlendState()
{
    Backend::BlendState blend;
    blend.colorOp_ = 0;	// func_add
    blend.alphaOp_ = 0;	// func_add
    blend.srcColor_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusDstAlpha);
    blend.dstColor_ = static_cast<uint32_t>(vk::BlendFactor::eOne);
    blend.srcAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusDstAlpha);
    blend.dstAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOne);
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
        "GaussianSplatting-preprocess",
        descriptorSetLayouts_,
        cs
    };
    preprocessPipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable().RequireComputePipeline(info);

    // cs = Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule(GetShaderPath() + "/sort.comp", ShaderType::Compute);
    // info = {
    //     "GaussianSplatting-sort",
    //     descriptorSetLayouts_,
    //     cs
    // };
    // sortPipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable().RequireComputePipeline(info);
}

bool GaussianRenderer::OnInit(Backend::DisplaySurface* surface)
{
    CreateGraphicsPipeline(surface->GetCurrentSwapSurface()->GetRenderPass());
    CreateComputePipeline();
    return true;
}

void GaussianRenderer::InitAuxiliaryBuffers(Scene* scene)
{
    preComputed_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer(
        { scene->totalPointCount_ * 64, BufferType::Storage });
}

void GaussianRenderer::RecordComputeCommands(Scene* scene)
{
    for (auto cmdBuffer : computeCmdBuffers_) {
        vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBuffer.begin(cmdBufferBeginInfo);

        // TODO: prepare buffer from scene
        preprocessPipeline_->BindStorageBuffers({ scene->ssboSplatData_ }, 0, 0);
        preprocessPipeline_->BindStorageBuffers({ preComputed_ }, 1, 0);
        preprocessPipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 1);
        preprocessPipeline_->BindUniformBuffers({ scene->uboModels_ }, 0, 2);
        preprocessPipeline_->BindUniformBuffers({ scene->uboCamera_ }, 0, 3);
        preprocessPipeline_->BindUniformBuffers({ surface_->GetScreenSizeBuffer() }, 0, 4);
        preprocessPipeline_->BindDescriptorSets(cmdBuffer);
        if (scene->totalPointCount_ != 0) {
            cmdBuffer.dispatch((scene->totalPointCount_ + 127) / 128, 1, 1);
        }
        // sortPipeline_->BindStorageBuffers({ preComputed_ }, 1, 0);
        // sortPipeline_->BindStorageBuffers({ scene->ssboSortedSplats_ }, 1, 1);
        // sortPipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 1);
        // sortPipeline_->BindDescriptorSets(cmdBuffer);
        // cmdBuffer.dispatch(1, 1, 1);
        cmdBuffer.end();
    }
}

void GaussianRenderer::OnRecordGraphicsCommands(Scene* scene, vk::CommandBuffer cmdBuffer)
{
    vk::DeviceSize offset = 0;
    cmdBuffer.bindVertexBuffers(0, vbo_->GetHandle(), offset);
    cmdBuffer.bindIndexBuffer(ibo_->GetHandle(), 0, vk::IndexType::eUint16);
    // TODO: prepare buffer from scene
    pipeline_->BindStorageBuffers({ scene->ssboSplatData_ }, 0, 0);
    pipeline_->BindStorageBuffers({ preComputed_ }, 1, 0);
    pipeline_->BindStorageBuffers({ scene->ssboSortedSplats_ }, 1, 1);
    pipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 1);
    pipeline_->BindUniformBuffers({ scene->uboModels_ }, 0, 2);
    pipeline_->BindUniformBuffers({ scene->uboCamera_ }, 0, 3);
    pipeline_->BindUniformBuffers({ surface_->GetScreenSizeBuffer() }, 0, 4);
    cmdBuffer.setViewport(0, { { 0.0f, 0.0f, static_cast<float>(surface_->GetWidth()), static_cast<float>(surface_->GetHeight()), 0.0f, 1.0f } });
    cmdBuffer.setScissor(0, { { { 0, 0 }, { surface_->GetWidth(), surface_->GetHeight() } } });
    pipeline_->BindDescriptorSets(cmdBuffer);
    cmdBuffer.drawIndexed(6, scene->totalPointCount_, 0, 0, 0);
    // cmdBuffer.drawIndexed(6, 128, 0, 0, 0);
    // cmdBuffer.drawIndexed(6, 1, 0, 0, 0);
}

void GaussianRenderer::SubmitGraphicsCommands()
{
    auto cmdBuffer = GetCurrentPresentCmdBuffer();
    auto queue = Backend::VkContext::GetInstance().AcquireGraphicsQueue(surface_->GetPresentQueueIdx());
    std::array<vk::Semaphore, 2> waitSemaphores{ preprocessComplete_, surface_->GetAcquireFrameSignalSemaphore() };
    std::array<vk::Semaphore, 2> signalSemaphores { surface_->GetPresentWaitSemaphore(), preprocessComplete_ };
    vk::SubmitInfo submitInfo{};
    std::array<vk::PipelineStageFlags, 2> waitStageMask{ vk::PipelineStageFlagBits::eVertexShader, vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setCommandBuffers(cmdBuffer)
        .setWaitSemaphores(waitSemaphores)
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphores(signalSemaphores);
    auto ret = queue.submit(submitInfo);
    if (ret != vk::Result::eSuccess) {
        XLOGE("Submit graphics commands failed, errCode: %d", ret);
        abort();
    }
    assert(ret == vk::Result::eSuccess);
}

void GaussianRenderer::SubmitComputeCommands()
{
    auto cmdBuffer = GetCurrentComputeCmdBuffer();

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
        abort();
    }
    assert(ret == vk::Result::eSuccess);
}

void GaussianRenderer::OnDrawFrame()
{
    this->SubmitComputeCommands();
    this->SubmitGraphicsCommands();
}

} // namespace X
