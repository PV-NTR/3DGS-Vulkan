#include "Renderer.hpp"

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
    context_.Init();
    Backend::VkResourceManager::GetInstance();
    return true;
}

void Renderer::Destroy()
{
    
}

bool Renderer::IsReady()
{
    return context_.IsReady();
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