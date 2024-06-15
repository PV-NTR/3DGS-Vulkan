#include "Renderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"
#include "common/LogCommon.hpp"

namespace X {

Renderer::Renderer(bool needCompute) : needCompute_(needCompute)
{

}

Renderer::~Renderer()
{

}

bool Renderer::Init(Backend::DisplaySurface* surface)
{
    assert(surface != nullptr);
    return this->OnInit(surface);
}

bool Renderer::OnInit(Backend::DisplaySurface* surface)
{
    surface_ = surface;
    auto device = Backend::VkContext::GetInstance().GetDevice();
    vk::CommandPoolCreateInfo poolCI {};
    poolCI.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    poolCI.setQueueFamilyIndex(surface->GetPresentQueueIdx());
    auto [presentRet, presentPoolUnique] = device.createCommandPoolUnique(poolCI);
    if (presentRet != vk::Result::eSuccess) {
        XLOGE("Create Present Pool failed, errCode: %d", presentRet);
        return false;
    }
    presentPool_.swap(presentPoolUnique);

    presentCmdBuffers_.resize(surface->GetSwapSurfaceCount());
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(*presentPool_).setCommandBufferCount(surface->GetSwapSurfaceCount());
    auto allocRet = device.allocateCommandBuffers(&allocInfo, presentCmdBuffers_.data());
    if (allocRet != vk::Result::eSuccess) {
        XLOGE("allocate present commandbuffer failed, errCode: %d", allocRet);
        return false;
    }

    if (needCompute_) {
        poolCI.setQueueFamilyIndex(Backend::VkContext::GetInstance().AcquireCurrentComputeQueue().first);
        auto [computeRet, computePoolUnique] = device.createCommandPoolUnique(poolCI);
        if (computeRet != vk::Result::eSuccess) {
            XLOGE("Create Compute Pool failed, errCode: %d", computeRet);
            return false;
        }
        computePool_.swap(computePoolUnique);

        computeCmdBuffers_.resize(surface->GetSwapSurfaceCount());
        allocInfo.setCommandPool(*computePool_).setCommandBufferCount(surface->GetSwapSurfaceCount());
        allocRet = device.allocateCommandBuffers(&allocInfo, computeCmdBuffers_.data());
        if (allocRet != vk::Result::eSuccess) {
            XLOGE("allocate compute commandbuffer failed, errCode: %d", allocRet);
            return false;
        }
    }
    return true;
}

void Renderer::Destroy()
{
    
}

bool Renderer::IsReady()
{
    return Backend::VkContext::GetInstance().IsReady();
}

void Renderer::UpdateScene(Scene* scene)
{
    if (scene->SceneChanged()) {
        this->OnUpdateScene(scene);
    }
}

void Renderer::OnUpdateScene(Scene* scene)
{
    if (surface_->Resized()) {
        this->RecordGraphicsCommands(scene);
        this->RecordComputeCommands(scene);
    }

    surface_->UpdateScreenSizeBuffer();
    if (scene->SceneChanged() && !scene->GetCamera().Updated()) {
        scene->UpdateData();
    }
}

void Renderer::DrawFrame()
{
    this->OnDrawFrame();
    surface_->Present();
    currentFrameIdx_ = surface_->NextFrame();
}

void Renderer::RecordGraphicsCommands(Scene* scene)
{
    auto cmdBuffer = GetCurrentPresentCmdBuffer();
    vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
    cmdBuffer.begin(cmdBufferBeginInfo);

    auto swapSurface = surface_->GetCurrentSwapSurface();
    vk::RenderPassBeginInfo beginInfo{};
    beginInfo.setRenderPass(swapSurface->GetRenderPass()->GetHandle())
        .setFramebuffer(swapSurface->GetFramebuffer()->get());
    cmdBuffer.beginRenderPass(beginInfo, { vk::SubpassContents::eInline });

    this->OnRecordGraphicsCommands(scene);

    cmdBuffer.endRenderPass();
    cmdBuffer.end();
}

void Renderer::SubmitGraphicsCommands()
{
    auto cmdBuffer = GetCurrentPresentCmdBuffer();

    auto queue = Backend::VkContext::GetInstance().AcquireGraphicsQueue(surface_->GetPresentQueueIdx());
    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setCommandBuffers(presentCmdBuffers_[currentFrameIdx_])
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&surface_->GetAcquireFrameSignalSemaphore())
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&surface_->GetPresentWaitSemaphore());
    queue.submit(submitInfo);
}

void Renderer::OnDrawFrame()
{
    this->SubmitGraphicsCommands();
}

} // namespace X