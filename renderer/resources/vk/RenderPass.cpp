#include "RenderPass.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"
#include "Image.hpp"

namespace X::Backend {

std::shared_ptr<RenderPass> RenderPass::Make()
{
    return std::shared_ptr<RenderPass>(new RenderPass());
}

std::shared_ptr<RenderPass> RenderPass::MakeDisplay(vk::Format targetFormat, bool depthStencil, bool load)
{
    return std::shared_ptr<RenderPass>(new RenderPass(targetFormat, depthStencil, load));
}

RenderPass::RenderPass(vk::Format targetFormat, bool depthStencil, bool load) noexcept
{
    this->AddAttachment(targetFormat, false, true, load);
    if (depthStencil) {
        this->AddAttachment(vk::Format::eD32SfloatS8Uint, true, false, load);
    }
    this->Init();
}

void RenderPass::AddAttachment(vk::Format format, bool depthStencil, bool present, bool load)
{
    attachments_.emplace_back();
    auto& attachment = attachments_.back();
    attachment.setFormat(format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setLoadOp(load ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore);

    if (depthStencil) {
        depthStencil_ = attachments_.size() - 1;
        attachment.setStencilLoadOp(load ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    } else {
        attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
        if (present) {
            display_ = attachments_.size() - 1;
            attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
        }
    }
}

vk::RenderPassCreateInfo RenderPass::SetupCreateInfo()
{
    subpassInfo_.colorRefs_.resize(depthStencil_ == UINT32_MAX ? attachments_.size() : attachments_.size() - 1);
    uint32_t i = 0;
    for (auto& colorRef : subpassInfo_.colorRefs_) {
        if (i == depthStencil_) {
            i++;
        }
        colorRef.setAttachment(i++).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    }

    if (depthStencil_ != UINT32_MAX) {
        subpassInfo_.depthRef_.setAttachment(depthStencil_).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    }

    subpassInfo_.subpassDesc_.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(subpassInfo_.colorRefs_);
    if (depthStencil_ != UINT32_MAX) {
        subpassInfo_.subpassDesc_.setPDepthStencilAttachment(&subpassInfo_.depthRef_);
    }

    // Subpass dependencies for layout transitions
    subpassInfo_.dependencies_.resize(2);
    subpassInfo_.dependencies_[0].setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
            vk::PipelineStageFlagBits::eLateFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
            vk::PipelineStageFlagBits::eLateFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentRead);

    subpassInfo_.dependencies_[1].setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead);

    vk::RenderPassCreateInfo renderPassCI{};
    renderPassCI.setAttachments(attachments_)
        .setSubpasses(subpassInfo_.subpassDesc_)
        .setDependencies(subpassInfo_.dependencies_);
    return renderPassCI;
}

void RenderPass::Init()
{
    if (inited) {
        XLOGV("RenderPass already inited!");
        return;
    }

    if (attachments_.empty()) {
        XLOGE("renderpass has no attachment, please add before init!");
        return;
    }

    auto renderPassCI = SetupCreateInfo();
    auto [ret, renderPassUnique] = VkContext::GetInstance().GetDevice().createRenderPassUnique(renderPassCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateRenderPass failed, errCode: %d", ret);
        return;
    }
    renderPassUnique_.swap(renderPassUnique);
    renderPass_ = *renderPassUnique_;
    inited = true;
}

std::vector<vk::Format> RenderPass::GetAttachemntFormats()
{
    std::vector<vk::Format> ret;
    for (const auto& attachment : attachments_) {
        ret.emplace_back(attachment.format);
    }
    return ret;
}

} // namespace X::Backend
