#pragma once

#include "VkResource.hpp"
#include "common/VkCommon.hpp"
#include "Image.hpp"
#include "RenderPass.hpp"

namespace X::Backend {

class Surface : public VkResourceBase {
public:
    [[nodiscard]] std::shared_ptr<Surface> FromThis();
    void AddAttachment(vk::Format format, bool depthStencil, bool present);
    void Init() { renderPass_->Init(); }

protected:
    Surface() noexcept = default;
    Surface(uint32_t width, uint32_t height) noexcept;
    explicit Surface(std::shared_ptr<RenderPass> renderPass) noexcept;
    void CreateAttachmentResources();

protected:
    std::shared_ptr<RenderPass> renderPass_;
    std::vector<std::shared_ptr<Image>> attachmentResources_;

    uint32_t width_ = UINT32_MAX;
    uint32_t height_ = UINT32_MAX;
};
    
} // namespace X::Backend
