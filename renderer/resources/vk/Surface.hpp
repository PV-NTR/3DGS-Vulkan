#pragma once

#include "VkResource.hpp"
#include "common/VkCommon.hpp"
#include "Image.hpp"
#include "RenderPass.hpp"

namespace X::Backend {

class Surface {
public:
    [[nodiscard]] static std::shared_ptr<Surface> Make(uint32_t width, uint32_t height, std::shared_ptr<RenderPass> renderPass = nullptr);
    [[nodiscard]] static std::shared_ptr<Surface> Make(const std::vector<std::shared_ptr<Image>>& attachmentResources);
    [[nodiscard]] std::shared_ptr<Surface> FromThis();
    void AddAttachment(vk::Format format, bool depthStencil, bool present);
    void Init();

    std::shared_ptr<RenderPass> GetRenderPass() const { return renderPass_; }
    std::shared_ptr<Framebuffer> GetFramebuffer() const { return framebuffer_; }

protected:
    Surface() noexcept = default;
    Surface(uint32_t width, uint32_t height, std::shared_ptr<RenderPass> renderPass = nullptr) noexcept;
    Surface(std::shared_ptr<RenderPass> renderPass, const std::vector<std::shared_ptr<Image>>& attachmentResources) noexcept;
    void BindAttachmentResources(const std::vector<std::shared_ptr<Image>>& attachmentResources);
    void CreateAttachmentResources();
    void CreateFramebuffer();

protected:
    friend class DisplaySurface;
    std::shared_ptr<RenderPass> renderPass_;
    std::vector<std::shared_ptr<Image>> attachmentResources_;
    std::shared_ptr<Framebuffer> framebuffer_;

    uint32_t width_ = UINT32_MAX;
    uint32_t height_ = UINT32_MAX;
};
    
} // namespace X::Backend
