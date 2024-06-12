#include "Surface.hpp"

#include "VkContext.hpp"
#include "VkResourceManager.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

std::shared_ptr<Surface> Surface::FromThis()
{
	if (!this->renderPass_->Inited()) {
		XLOGW("RenderPass not inited, please add attachment and init it first!");
		return nullptr;
	}
	return std::shared_ptr<Surface>(new Surface(this->renderPass_));
}

Surface::Surface(uint32_t width, uint32_t height) noexcept
	: width_(width), height_(height)
{
	renderPass_ = RenderPass::Make();
}

Surface::Surface(std::shared_ptr<RenderPass> renderPass) noexcept
{
	renderPass_ = renderPass;
	this->CreateAttachmentResources();
}

void Surface::AddAttachment(vk::Format format, bool depthStencil, bool present)
{
	attachmentResources_.emplace_back(VkResourceManager::GetInstance().GetImageManager().RequireImage({ width_, height_, format, depthStencil }));
	renderPass_->AddAttachment(format, depthStencil, present);
}

void Surface::CreateAttachmentResources()
{
	auto formats = renderPass_->GetAttachemntFormats();
	uint32_t i = 0;
	for (auto format : formats) {
		attachmentResources_.emplace_back(VkResourceManager::GetInstance().GetImageManager().RequireImage({ width_, height_, format,
			i++ == renderPass_->DepthStencilAttachmentID()}));
	}
}

} // namespace X::Backend
