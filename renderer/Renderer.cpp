#include "Renderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"
#include "common/LogCommon.hpp"

namespace X {

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

bool Renderer::Init(Backend::Surface* surface)
{
    assert(surface != nullptr);
    surface_ = surface;
    return this->OnInit(surface);
}

bool Renderer::OnInit(Backend::Surface* surface)
{
    vk::CommandPoolCreateInfo poolCI {};
    poolCI.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(Backend::VkContext::GetInstance().AcquireCurrentComputeQueue().first);
    auto [computeRet, computePoolUnique] = Backend::VkContext::GetInstance().GetDevice().createCommandPoolUnique(poolCI);
    if (computeRet != vk::Result::eSuccess) {
        XLOGE("Create Compute Pool failed, errCode: %d", computeRet);
        return false;
    }
    computePoolUnique_.swap(computePoolUnique);
    computePool_ = *computePoolUnique_;

    poolCI.setQueueFamilyIndex(surface->GetPresentQueueIdx());
    auto [presentRet, presentPoolUnique] = Backend::VkContext::GetInstance().GetDevice().createCommandPoolUnique(poolCI);
    if (presentRet != vk::Result::eSuccess) {
        XLOGE("Create Present Pool failed, errCode: %d", presentRet);
        return false;
    }
    presentPoolUnique_.swap(presentPoolUnique);
    presentPool_ = *presentPoolUnique_;

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

}

void Renderer::DrawFrame()
{
    this->OnDrawFrame();
}

void Renderer::OnDrawFrame()
{

}

} // namespace X