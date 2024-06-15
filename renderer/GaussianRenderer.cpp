#include "GaussianRenderer.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"

namespace X {

const std::vector<float> GaussianRenderer::vboData_ = {
	-2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f
};

GaussianRenderer::GaussianRenderer()
	: Renderer(true)
{
	vbo_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ vboData_.size() * sizeof(float), BufferType::Vertex });
	vbo_->Update(vboData_.data(), vboData_.size() * sizeof(float), 0);
	SetDescriptorSetLayouts();
	SetBlendState();
}

void GaussianRenderer::SetDescriptorSetLayouts()
{
	descriptorSetLayouts_.resize(2, Backend::DescriptorSetLayout::Make());
	descriptorSetLayouts_[0]->AddDescriptorBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute, 2);
	descriptorSetLayouts_[0]->AddDescriptorBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute, 4);
	descriptorSetLayouts_[0]->Update();
	descriptorSetLayouts_[1]->AddDescriptorBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute, 1);
	descriptorSetLayouts_[1]->Update();
}

void GaussianRenderer::SetBlendState()
{
	Backend::BlendState blend;
	blend.colorOp_ = 0;	// func_add
	blend.alphaOp_ = 0;	// func_add
	blend.srcColor_ = static_cast<uint32_t>(vk::BlendFactor::eOne);
	blend.dstColor_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusSrcAlpha);
	blend.srcAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOne);
	blend.dstAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusSrcAlpha);
	blend_.emplace(blend);
}

void GaussianRenderer::CreateGraphicsPipeline(std::shared_ptr<Backend::RenderPass> renderPass)
{
	std::shared_ptr<Backend::ShaderModule> vs =
		Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule("Shaders/gsplat.vert", ShaderType::Vertex);
	std::shared_ptr<Backend::ShaderModule> fs =
		Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule("Shaders/gsplat.frag", ShaderType::Fragment);
	Backend::GraphicsPipelineInfo info = {
		"GaussianSplatting",
		renderPass,
		descriptorSetLayouts_,
		blend_,
		vs,
		fs
	};
	pipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable().RequireGraphicsPipeline(info);
}

void GaussianRenderer::CreateComputePipeline()
{
	std::shared_ptr<Backend::ShaderModule> cs =
		Backend::VkResourceManager::GetInstance().GetShaderManager().AddShaderModule("Shaders/precompute.comp", ShaderType::Compute);
	Backend::ComputePipelineInfo info = {
		"GaussianSplatting",
		descriptorSetLayouts_,
		cs
	};
	computePipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable().RequireComputePipeline(info);
}

bool GaussianRenderer::OnInit(Backend::DisplaySurface* surface)
{
	bool status = Renderer::OnInit(surface);
	CreateGraphicsPipeline(surface->GetCurrentSwapSurface()->GetRenderPass());
	return status;
	// CreateComputePipeline();
}

void GaussianRenderer::RecordComputeCommands(Scene* scene)
{
	auto cmdBuffer = GetCurrentComputeCmdBuffer();
	// TODO: prepare buffer from scene
	computePipeline_->BindStorageBuffers(scene->ssboExtrinsic_, 0, 0);
	computePipeline_->BindStorageBuffers(scene->ssboIntrinsic_, 0, 1);
	computePipeline_->BindStorageBuffers(preComputed_, 1, 0);
	computePipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 2);
	computePipeline_->BindUniformBuffers(scene->uboModels_, 0, 3);
	computePipeline_->BindUniformBuffers({ scene->uboCamera_ }, 0, 4);
	computePipeline_->BindUniformBuffers({ surface_->GetScreenSizeBuffer() }, 0, 5);
	computePipeline_->BindDescriptorSets(cmdBuffer);
}

void GaussianRenderer::OnRecordGraphicsCommands(Scene* scene)
{
	auto cmdBuffer = GetCurrentPresentCmdBuffer();
	cmdBuffer.bindVertexBuffers(0, vbo_->GetHandle(), {});
	// TODO: prepare buffer from scene
	pipeline_->BindStorageBuffers(scene->ssboExtrinsic_, 0, 0);
	pipeline_->BindStorageBuffers(scene->ssboIntrinsic_, 0, 1);
	pipeline_->BindStorageBuffers(preComputed_, 1, 0);
	pipeline_->BindUniformBuffers({ scene->uboPrefixSums_ }, 0, 2);
	pipeline_->BindUniformBuffers(scene->uboModels_, 0, 3);
	pipeline_->BindUniformBuffers({ scene->uboCamera_ }, 0, 4);
	pipeline_->BindUniformBuffers({ surface_->GetScreenSizeBuffer() }, 0, 5);
	pipeline_->BindDescriptorSets(cmdBuffer);
}

void GaussianRenderer::SubmitGraphicsCommands()
{
	auto cmdBuffer = GetCurrentPresentCmdBuffer();

	auto queue = Backend::VkContext::GetInstance().AcquireGraphicsQueue(surface_->GetPresentQueueIdx());
	vk::SubmitInfo submitInfo{};
	std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.setCommandBuffers(cmdBuffer)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&preprocessComplete_)
		.setWaitDstStageMask(waitStageMask)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&surface_->GetPresentWaitSemaphore());
	queue.submit(submitInfo);
}

void GaussianRenderer::SubmitComputeCommands()
{
	auto cmdBuffer = GetCurrentComputeCmdBuffer();

	auto queue = Backend::VkContext::GetInstance().AcquireCurrentComputeQueue().second;
	vk::SubmitInfo submitInfo{};
	std::vector<vk::PipelineStageFlags> waitStageMask{ vk::PipelineStageFlagBits::eVertexShader };
	submitInfo.setCommandBuffers(cmdBuffer)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&surface_->GetPresentWaitSemaphore())	// wait semaphore of last frame, right?
		.setWaitDstStageMask(waitStageMask)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&preprocessComplete_);
	queue.submit(submitInfo);
}

void GaussianRenderer::OnDrawFrame()
{
	this->SubmitComputeCommands();
	this->SubmitGraphicsCommands();
}

} // namespace X
