#pragma once

#include "VmaObject.hpp"

namespace X::Backend {

class Image;

struct VmaImageInfo {
    uint32_t width_;
    uint32_t height_;
    vk::Format format_;
    vk::ImageUsageFlags usage_;
    // TODO: consider ycbcr and mipmap
};

struct VmaImageState {
    vk::ImageLayout layout_ = vk::ImageLayout::eUndefined;
    vk::AccessFlags accessType_ = vk::AccessFlagBits::eNone;
    vk::PipelineStageFlags accessStage_ = vk::PipelineStageFlagBits::eTopOfPipe;
    uint32_t queueFamilyIndex_ = vk::QueueFamilyIgnored;
};

class VmaImage : public VmaObject {
public:
    VmaImage() noexcept = default;
    VmaImage(VmaAllocator allocator, const VmaImageInfo& info) noexcept;
    // from external
    VmaImage(vk::Image image, VmaImageState&& state) noexcept;
    ~VmaImage() noexcept;

    VmaImage(VmaImage&& other) noexcept;
    VmaImage& operator=(VmaImage&& other) noexcept;
    vk::Image GetHandle() { return handle_; };
    void Barrier(vk::CommandBuffer cmdBuffer, VmaImageState&& state, vk::ImageAspectFlags aspectMask,
        vk::DependencyFlags flags = {});

protected:
    void Destroy() noexcept;

private:
    vk::Image handle_;
    VmaImageState state_;
    bool external_ = false;
};

} // namespace X::Backend
