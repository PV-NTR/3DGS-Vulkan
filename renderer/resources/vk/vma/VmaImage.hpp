#pragma once

#include "VmaObject.hpp"

namespace X::Backend {

class Image;

struct VmaImageInfo {
    uint32_t width_;
    uint32_t height_;
    vk::Format format_;
    vk::ImageUsageFlags usage_;
    vk::MemoryPropertyFlags memProps_;
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
    void Barrier(vk::CommandBuffer cmdBuffer, VmaImageState&& state, vk::ImageAspectFlags aspectMask, vk::DependencyFlags flags = {}) {
        if (state_.layout_ == state.layout_ && state_.accessType_ == state.accessType_ &&
            state_.accessStage_ == state.accessStage_ && state_.queueFamilyIndex_ == state.queueFamilyIndex_) {
            return;
        }
        vk::ImageMemoryBarrier barrier{};
        barrier.setImage(handle_)
            .setSrcAccessMask(state_.accessType_)
            .setDstAccessMask(state.accessType_)
            .setOldLayout(state_.layout_)
            .setNewLayout(state.layout_)
            .setSrcQueueFamilyIndex(state_.queueFamilyIndex_)
            .setDstQueueFamilyIndex(state.queueFamilyIndex_)
            .setSubresourceRange({ aspectMask, 0, 1, 0, 1 });
        cmdBuffer.pipelineBarrier(state_.accessStage_, state.accessStage_, flags, {}, {}, barrier);
        state_ = std::move(state);
    }

protected:
    void Destroy() noexcept;

private:
    vk::Image handle_;
    VmaImageState state_;
    bool external_ = false;
};

} // namespace X::Backend
