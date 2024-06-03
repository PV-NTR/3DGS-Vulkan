#pragma once

#include "VkResourceBase.hpp"
#include "common/VkCommon.hpp"

namespace X::Backend {

class RenderPass : public VkResourceBase {
public:
    RenderPass(vk::Format targetFormat = vk::Format::eR8G8B8A8Unorm, bool load = false) noexcept;
    virtual ~RenderPass() = default;
    vk::RenderPass GetHandle() { return renderPass_; }

private:
    vk::UniqueRenderPass renderPassUnique_;
    vk::RenderPass renderPass_;
};

} // namespace X::Backend
