#include "RenderPass.hpp"

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

RenderPass::RenderPass(vk::Format targetFormat, bool load) noexcept
{
    std::array<vk::AttachmentDescription, 2> attachments;
    // color attachment
    attachments[0].setFormat(targetFormat)
        .setLoadOp(load ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    // Depth attachment
    // TODO: get valid format instead of hard code
    attachments[1].setFormat(vk::Format::eD32SfloatS8Uint)
        .setLoadOp(load ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(load ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    
    std::array<vk::AttachmentReference, 1> colorRef;
    colorRef[0].setAttachment(0).setLayout(vk::ImageLayout::eAttachmentOptimal);

    vk::AttachmentReference depthRef {};
    depthRef.setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpassDesc {};
    subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(colorRef)
        .setPDepthStencilAttachment(&depthRef);

    // Subpass dependencies for layout transitions
    std::array<vk::SubpassDependency, 2> dependencies;
    dependencies[0].setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead);

    dependencies[1].setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite| vk::AccessFlagBits::eColorAttachmentRead);

    vk::RenderPassCreateInfo renderPassCI {};
    renderPassCI.setAttachments(attachments)
        .setSubpasses(subpassDesc)
        .setDependencies(dependencies);

    auto [ret, renderPassUnique] = VkContext::GetInstance().GetDevice().createRenderPassUnique(renderPassCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateRenderPass failed, errCode: %d", ret);
        return;
    }
    renderPassUnique_.swap(renderPassUnique);
    renderPass_ = *renderPassUnique_;
}
    
} // namespace X::Backend
