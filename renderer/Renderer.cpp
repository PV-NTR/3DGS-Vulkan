#include "Renderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"

namespace X {

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

bool Renderer::Init()
{
    Backend::VkContext::GetInstance().Init();
    Backend::VkResourceManager::GetInstance();
    return true;
}

void Renderer::Destroy()
{
    
}

bool Renderer::IsReady()
{
    return Backend::VkContext::GetInstance().IsReady();
}

void Renderer::UpdateScene(const Scene& scene)
{
    if (scene.SceneChanged()) {
        
    }
}

void Renderer::DrawFrame()
{

}

} // namespace X