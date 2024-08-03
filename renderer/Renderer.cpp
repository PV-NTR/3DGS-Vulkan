#include "Renderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"
#include "common/LogCommon.hpp"

namespace X {

Renderer::Renderer(bool needCompute) : needCompute_(needCompute)
{
    fence_ = Backend::VkContext::GetInstance().GetDevice().createFence({}).value;
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
    // TODO: correct usage of auxiliaryInited
    if (auxiliaryInited && scene->ObjectChanged()) {
        this->InitAuxiliaryBuffers(scene);
    }
    this->OnUpdateScene(scene);
}

void Renderer::OnUpdateScene(Scene* scene)
{
    if (surface_->Changed()) {
        this->RecordGraphicsCommands(scene);
        this->RecordComputeCommands(scene);
    }
    if (scene->SceneChanged()) {
        scene->UpdateData(surface_);
    }
    if (surface_->Changed()) {
        surface_->UpdateScreenSizeBuffer();
    }
}

void Renderer::DrawFrame()
{
    this->OnDrawFrame();
    // Backend::VkContext::GetInstance().GetDevice().waitForFences(fence_, vk::True, 100000000);
    surface_->Present();
    // Backend::VkContext::GetInstance().GetDevice().resetFences(fence_);
}


bool Renderer::AllocateCommandBuffers()
{
    auto device = Backend::VkContext::GetInstance().GetDevice();

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(Backend::VkContext::GetInstance().GetPresentCmdPool())
        .setCommandBufferCount(surface_->GetSwapSurfaceCount());
    auto [allocRet, presentCmdBuffers] = device.allocateCommandBuffersUnique(allocInfo);
    if (allocRet != vk::Result::eSuccess) {
        XLOGE("allocate present commandbuffer failed, errCode: %d", allocRet);
        return false;
    }
    for (auto&& presentCmdBuffer : presentCmdBuffers) {
        presentCmdBuffers_.emplace_back(std::make_shared<Backend::CommandBuffer>(std::move(presentCmdBuffer)));
    }

    if (needCompute_) {
        allocInfo.setCommandPool(Backend::VkContext::GetInstance().GetComputeCmdPool())
            .setCommandBufferCount(surface_->GetSwapSurfaceCount());
        auto [allocRet, computeCmdBuffers] = device.allocateCommandBuffersUnique(allocInfo);
        if (allocRet != vk::Result::eSuccess) {
            XLOGE("allocate compute commandbuffer failed, errCode: %d", allocRet);
            return false;
        }
        for (auto&& computeCmdBuffer : computeCmdBuffers) {
            computeCmdBuffers_.emplace_back(std::make_shared<Backend::CommandBuffer>(std::move(computeCmdBuffer)));
        }
    }
    return true;
}

void Renderer::RecordGraphicsCommands(Scene* scene)
{
    for (uint32_t i = 0; i < presentCmdBuffers_.size(); i++) {
        auto cmdBuffer = presentCmdBuffers_[i]->get();
        cmdBuffer.reset();
        vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
        auto ret = cmdBuffer.begin(cmdBufferBeginInfo);
        if (ret != vk::Result::eSuccess) {
            XLOGE("BeginCommandBuffer failed, errCode: %d", ret);
            return;
        }

        auto swapSurface = surface_->GetSwapSurfaces()[i];
        vk::RenderPassBeginInfo beginInfo{};
        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].setColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        clearValues[1].setDepthStencil({ 1.0f, 0 });
        beginInfo.setRenderPass(swapSurface->GetRenderPass()->GetHandle())
            .setFramebuffer(swapSurface->GetFramebuffer()->get())
            .setClearValues(clearValues)
            .setRenderArea({ {0, 0}, {surface_->GetWidth(), surface_->GetHeight()} });
        cmdBuffer.beginRenderPass(beginInfo, { vk::SubpassContents::eInline });
        this->OnRecordGraphicsCommands(scene, presentCmdBuffers_[i]);
        scene->overlay_.PrepareDrawCommands(presentCmdBuffers_[i]);

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
    auto waitSemaphore = surface_->GetAcquireFrameSignalSemaphore();
    auto signalSemaphore = surface_->GetPresentWaitSemaphore();
    submitInfo.setCommandBuffers(cmdBuffer->get())
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&waitSemaphore)
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&signalSemaphore);
    auto ret = queue.submit(submitInfo);
    // auto ret = queue.submit(submitInfo, fence_);
    if (ret != vk::Result::eSuccess) {
        XLOGE("Submit graphics commands failed, errCode: %d", ret);
        // abort();
    }
    assert(ret != vk::Result::eErrorDeviceLost);
}

void Renderer::OnDrawFrame()
{
    this->SubmitGraphicsCommands();
}

} // namespace X