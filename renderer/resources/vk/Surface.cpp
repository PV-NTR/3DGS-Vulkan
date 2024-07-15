#include "Surface.hpp"

#include "VkContext.hpp"
#include "VkResourceManager.hpp"
#include "common/LogCommon.hpp"
#include "ImageView.hpp"

namespace X::Backend {

std::shared_ptr<Surface> Surface::Make(uint32_t width, uint32_t height, std::shared_ptr<RenderPass> renderPass)
{
	return std::shared_ptr<Surface>(new Surface(width, height, renderPass));
}

std::shared_ptr<Surface> Surface::Make(const std::vector<std::shared_ptr<Image>>& attachmentResources)
{
	assert(attachmentResources.size() == 2);
	auto renderPass = RenderPass::MakeDisplay();
	assert(renderPass->Inited());
	return std::shared_ptr<Surface>(new Surface(std::move(renderPass), attachmentResources));
}

std::shared_ptr<Surface> Surface::Make(std::shared_ptr<RenderPass> renderPass, const std::vector<std::shared_ptr<Image>>& attachmentResources)
{
	assert(attachmentResources.size() == 2);
	assert(renderPass->Inited());
	return std::shared_ptr<Surface>(new Surface(std::move(renderPass), attachmentResources));
}

std::shared_ptr<Surface> Surface::FromThis()
{
	if (!this->renderPass_->Inited()) {
		XLOGW("RenderPass not inited, please add attachment and init it first!");
		return nullptr;
	}
	return std::shared_ptr<Surface>(new Surface(this->width_, this->height_, this->renderPass_));
}

Surface::Surface(uint32_t width, uint32_t height, std::shared_ptr<RenderPass> renderPass) noexcept
	: width_(width), height_(height)
{
	if (renderPass) {
		renderPass_ = renderPass;
		if (renderPass_->Inited()) {
			this->CreateAttachmentResources();
		}
	} else {
		renderPass_ = RenderPass::Make();
	}
}

Surface::Surface(std::shared_ptr<RenderPass> renderPass,
	const std::vector<std::shared_ptr<Image>>& attachmentResources) noexcept
{
	renderPass_ = renderPass;
	BindAttachmentResources(attachmentResources);
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

void Surface::BindAttachmentResources(const std::vector<std::shared_ptr<Image>>& attachmentResources)
{
	width_ = attachmentResources[0]->GetInfo().width_;
	height_ = attachmentResources[0]->GetInfo().height_;
	attachmentResources_ = attachmentResources;
}

void Surface::CreateFramebuffer()
{
	vk::FramebufferCreateInfo fbCI{};
	std::vector<vk::ImageView> attachments;
	attachments.reserve(attachmentResources_.size());
	for (const auto& attachmentResource : attachmentResources_) {
		if (attachmentResource) {
			attachments.emplace_back(attachmentResource->GetView()->GetHandle());
		}
	}
	fbCI.setRenderPass(renderPass_->GetHandle())
		.setAttachments(attachments)
		.setLayers(1)
		.setWidth(width_)
		.setHeight(height_);
	auto [ret, fbUnique] = VkContext::GetInstance().GetDevice().createFramebufferUnique(fbCI);
	if (ret != vk::Result::eSuccess) {
		XLOGE("CreateFramebuffer failed, errCode: %d", ret);
		return;
	}
	framebuffer_ = std::make_shared<Framebuffer>(std::move(fbUnique));
	for (const auto& attachmentResource : attachmentResources_) {
		if (attachmentResource) {
			framebuffer_->DependOn(attachmentResource->GetView());
		}
	}
}

void Surface::Init()
{
	renderPass_->Init();
	CreateFramebuffer();
}

} // namespace X::Backend
