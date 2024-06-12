#include "GaussianRenderer.hpp"

namespace X {

GaussianRenderer::GaussianRenderer()
{
	renderPass_ = Backend::RenderPass::Make();
}

void GaussianRenderer::RecordGraphicsCommands()
{
	auto cmdBuffer_ = GetCurrentPresentCmdBuffer();
	//vk::RenderPassBeginInfo beginInfo{};
	//beginInfo.setRenderPass(renderPass_->GetHandle())
	//	.setFramebuffer(surface_->GetFramebuffer());
	//cmdBuffer_.beginRenderPass(beginInfo);
}

} // namespace X
