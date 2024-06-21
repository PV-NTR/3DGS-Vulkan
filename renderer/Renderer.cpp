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

    surface_->UpdateScreenSizeBuffer();
    if (scene->SceneChanged() && !scene->GetCamera().Updated()) {
        scene->UpdateData();
    }
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
    allocInfo.setCommandPool(Backend::VkContext::GetInstance().GetPresentCmdPool()).setCommandBufferCount(1);
    auto allocRet = device.allocateCommandBuffers(&allocInfo, &presentCmdBuffer_);
    if (allocRet != vk::Result::eSuccess) {
        XLOGE("allocate present commandbuffer failed, errCode: %d", allocRet);
        return false;
    }

    if (needCompute_) {
        allocInfo.setCommandPool(Backend::VkContext::GetInstance().GetComputeCmdPool()).setCommandBufferCount(1);
        allocRet = device.allocateCommandBuffers(&allocInfo, &computeCmdBuffer_);
        if (allocRet != vk::Result::eSuccess) {
            XLOGE("allocate compute commandbuffer failed, errCode: %d", allocRet);
            return false;
        }
    }
    return true;
}

void Renderer::RecordGraphicsCommands(Scene* scene)
{
    // TODO: restore to commandbuffers, record all
    auto cmdBuffer = GetPresentCmdBuffer();
    vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
    cmdBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    cmdBuffer.begin(cmdBufferBeginInfo);

    surface_->GetCurrentDisplayImage()->Barrier(cmdBuffer,
        { vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput },
        vk::ImageAspectFlagBits::eColor);

    auto swapSurface = surface_->GetCurrentSwapSurface();
    vk::RenderPassBeginInfo beginInfo{};
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].setColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    clearValues[1].setDepthStencil({1.0f, 0});
    beginInfo.setRenderPass(swapSurface->GetRenderPass()->GetHandle())
        .setFramebuffer(swapSurface->GetFramebuffer()->get())
        .setClearValues(clearValues)
        .setRenderArea({ {0, 0}, {surface_->GetWidth(), surface_->GetHeight()} });
    cmdBuffer.beginRenderPass(beginInfo, { vk::SubpassContents::eInline });
    this->OnRecordGraphicsCommands(scene);

    cmdBuffer.endRenderPass();
    cmdBuffer.end();
}

void Renderer::SubmitGraphicsCommands()
{
    auto cmdBuffer = GetPresentCmdBuffer();

    auto queue = Backend::VkContext::GetInstance().AcquireGraphicsQueue(surface_->GetPresentQueueIdx());
    vk::SubmitInfo submitInfo{};
    std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setCommandBuffers(presentCmdBuffer_)
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