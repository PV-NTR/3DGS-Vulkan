#pragma once

#include "VkResourceBase.hpp"
#include "vma/VmaImage.hpp"
#include "resources/Enums.hpp"
#include "ImageView.hpp"

namespace X::Backend {

class ImageManager;

struct ImageInfo {
    uint32_t width_;
    uint32_t height_;
    vk::Format format_;
    bool depthStencil_ = false;
    // TODO: support other colortype
    // ColorType colorType_;
};

class Image : public VkResourceBase, public std::enable_shared_from_this<Image> {
private:
    friend class ImageManager;
    friend class DisplaySurface;

public:
    virtual ~Image() noexcept = default;
    vk::Image GetHandle() { return image_.GetHandle(); };
    const ImageInfo& GetInfo() { return info_; }
    std::shared_ptr<ImageView> GetView() { return view_; }
    void Barrier(vk::CommandBuffer cmdBuffer, VmaImageState&& state, vk::ImageAspectFlags aspectMask, vk::DependencyFlags flags = {})
    {
        return image_.Barrier(cmdBuffer, std::move(state), aspectMask, flags);
    }

private:
    Image(VmaAllocator allocator, const ImageInfo& info) noexcept;
    // external image, only for swapchain
    Image(vk::Image image, const ImageInfo& info, VmaImageState&& state) noexcept;
    Image(Image&& other) noexcept : image_(std::move(other.image_)), info_(other.info_), view_(std::move(other.view_)) {}

    void CreateView();

private:
    VmaImage image_;
    ImageInfo info_;
    std::shared_ptr<ImageView> view_;
};

} // namespace X::Backend
