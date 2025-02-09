#pragma once

#include "VkResource.hpp"
#include "vma/VmaImage.hpp"
#include "resources/Enums.hpp"
#include "ImageView.hpp"

namespace X::Backend {

class ImageManager;

struct ImageInfo {
    uint32_t width_;
    uint32_t height_;
    vk::Format format_ = vk::Format::eR8G8B8A8Unorm;
    bool depthStencil_ = false;
    // TODO: support other colortype
    // ColorType colorType_;
};

class Image : public VkResourceBase, public std::enable_shared_from_this<Image> {
private:
    friend class ImageManager;
    friend class DisplaySurface;

public:
    ~Image() override = default;
    vk::Image GetHandle() { return image_.GetHandle(); };
    const ImageInfo& GetInfo() { return info_; }
    std::shared_ptr<ImageView> GetView() { return view_; }
    // TODO: complete this
    void Update(/* params */) {}
    void Barrier(std::shared_ptr<CommandBuffer> cmdBuffer, VmaImageState&& state, vk::ImageAspectFlags aspectMask,
        vk::DependencyFlags flags = {})
    {
        return image_.Barrier(cmdBuffer->get(), std::move(state), aspectMask, flags);
    }

private:
    Image(VmaAllocator allocator, const ImageInfo& info) noexcept;
    // external image, only for swapchain
    Image(vk::Image image, const ImageInfo& info, VmaImageState&& state) noexcept;
    Image(Image&& other) noexcept : image_(std::move(other.image_)), info_(other.info_),
        view_(std::move(other.view_)) {}
    Image& operator=(Image&& other) = delete;

    void CreateView();

private:
    VmaImage image_;
    ImageInfo info_;
    std::shared_ptr<ImageView> view_;
};

} // namespace X::Backend
