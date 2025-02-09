#pragma once

#include "VkResourceBase.hpp"
#include "common/VkCommon.hpp"

namespace X::Backend {

struct RenderPassInfo {
    // TODO: complete this, init renderpass with info instead of vk structure
};

class RenderPass : public VkResourceBase {
public:
    [[nodiscard]] static std::shared_ptr<RenderPass> Make();
    [[nodiscard]] static std::shared_ptr<RenderPass> MakeDisplay(vk::Format targetFormat = vk::Format::eR8G8B8A8Unorm,
        bool depthStencil = false, bool load = false);
    ~RenderPass() override = default;
    vk::RenderPass GetHandle() { return renderPass_; }
    void AddAttachment(vk::Format format, bool depthStencil = false, bool present = false, bool load = false);
    void Init();
    bool Inited() const { return inited; }
    std::vector<vk::Format> GetAttachemntFormats();
    uint32_t DepthStencilAttachmentID() const { return depthStencil_; }

protected:
    RenderPass() noexcept = default;
    RenderPass(vk::Format targetFormat, bool depthStencil, bool load) noexcept;
    vk::RenderPassCreateInfo SetupCreateInfo();

private:
    vk::UniqueRenderPass renderPassUnique_;
    vk::RenderPass renderPass_;
    std::vector<vk::AttachmentDescription> attachments_ {};
    struct {
        std::vector<vk::AttachmentReference> colorRefs_;
        vk::AttachmentReference depthRef_;
        vk::SubpassDescription subpassDesc_;
        std::vector<vk::SubpassDependency> dependencies_;
    } subpassInfo_;
    // we only support at most 1 display image and 1 depthStencil image for one render pass!
    uint32_t display_ = UINT32_MAX;
    uint32_t depthStencil_ = UINT32_MAX;
    bool inited = false;
};

} // namespace X::Backend
