#include "GaussianRenderer.hpp"

#include "resources/vk/VkContext.hpp"

namespace X {

GaussianRenderer::GaussianRenderer()
{
}

void GaussianRenderer::RecordComputeCommands(Scene* scene)
{

}

void GaussianRenderer::OnRecordGraphicsCommands(Scene* scene)
{
	auto cmdBuffer = GetCurrentPresentCmdBuffer();
}

void GaussianRenderer::OnDrawFrame()
{

}

} // namespace X
