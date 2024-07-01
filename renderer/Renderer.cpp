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
    surface_ = surface;

    return this->AllocateCommandBuffers() && this->OnInit(surface);
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
    if (auxiliaryInited && scene->ObjectChanged()) {
        this->InitAuxiliaryBuffers(scene);
    }
    // if (scene->SceneChanged()) {
        this->OnUpdateScene(scene);
    // }
}

void Renderer::OnUpdateScene(Scene* scene)
{
    if (surface_->Resized()) {
        this->RecordGraphicsCommands(scene);
        this->RecordComputeCommands(scene);
    }

    //if (scene->SceneChanged() && !scene->GetCamera().Updated()) {
        scene->UpdateData(surface_);
    //}
    surface_->UpdateScreenSizeBuffer();
}

void Renderer::DrawFrame()
{
    this->OnDrawFrame();
    surface_->Present();
    surface_->NextFrame();
}


bool Renderer::AllocateCommandBuffers()
{
    auto device = Backend::VkContext::GetInstance().GetDevice();

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(Backend::VkContext::GetInstance().GetPresentCmdPool()).setCommandBufferCount(surface_->GetSwapSurfaceCount());
    presentCmdBuffers_.resize(surface_->GetSwapSurfaceCount());
    auto allocRet = device.allocateCommandBuffers(&allocInfo, presentCmdBuffers_.data());
    if (allocRet != vk::Result::eSuccess) {
        XLOGE("allocate present commandbuffer failed, errCode: %d", allocRet);
        return false;
    }

    if (needCompute_) {
        allocInfo.setCommandPool(Backend::VkContext::GetInstance().GetComputeCmdPool()).setCommandBufferCount(surface_->GetSwapSurfaceCount());
        computeCmdBuffers_.resize(surface_->GetSwapSurfaceCount());
        allocRet = device.allocateCommandBuffers(&allocInfo, computeCmdBuffers_.data());
        if (allocRet != vk::Result::eSuccess) {
            XLOGE("allocate compute commandbuffer failed, errCode: %d", allocRet);
            return false;
        }
    }
    return true;
}

void Renderer::RecordGraphicsCommands(Scene* scene)
{
    for (uint32_t i = 0; i < presentCmdBuffers_.size(); i++) {
        auto cmdBuffer = presentCmdBuffers_[i];
        vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
        auto ret = cmdBuffer.begin(cmdBufferBeginInfo);
        if (ret != vk::Result::eSuccess) {
            XLOGE("BeginCommandBuffer failed, errCode: %d", ret);
            return;
        }

        auto swapSurface = surface_->GetSwapSurfaces()[i];
        vk::RenderPassBeginInfo beginInfo{};
        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].setColor({ 0.3f, 0.0f, 0.0f, 0.5f });
        clearValues[1].setDepthStencil({ 1.0f, 0 });
        beginInfo.setRenderPass(swapSurface->GetRenderPass()->GetHandle())
            .setFramebuffer(swapSurface->GetFramebuffer()->get())
            .setClearValues(clearValues)
            .setRenderArea({ {0, 0}, {surface_->GetWidth(), surface_->GetHeight()} });
        cmdBuffer.beginRenderPass(beginInfo, { vk::SubpassContents::eInline });
        this->OnRecordGraphicsCommands(scene, cmdBuffer);

        cmdBuffer.endRenderPass();
        ret = cmdBuffer.end();
        if (ret != vk::Result::eSuccess) {
            XLOGE("EndCommandBuffer failed, errCode: %d", ret);
            return;
        }
    }
}

void Renderer::SubmitGraphicsCommands()
{
    auto cmdBuffer = GetCurrentPresentCmdBuffer();

    auto queue = Backend::VkContext::GetInstance().AcquireGraphicsQueue(surface_->GetPresentQueueIdx());
    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setCommandBuffers(cmdBuffer)
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&surface_->GetAcquireFrameSignalSemaphore())
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&surface_->GetPresentWaitSemaphore());
    auto ret = queue.submit(submitInfo);
    if (ret != vk::Result::eSuccess) {
        XLOGE("Submit graphics commands failed, errCode: %d", ret);
        abort();
    }
}

void Renderer::OnDrawFrame()
{
    this->SubmitGraphicsCommands();
}

} // namespace X