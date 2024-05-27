#include "renderer.hpp"

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