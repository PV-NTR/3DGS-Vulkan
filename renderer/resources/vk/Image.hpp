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
    // TODO: support other colortype
    // ColorType colorType_;
};

class Image : public VkResourceBase, public std::enable_shared_from_this<Image> {
private:
    friend class ImageManager;

public:
    virtual ~Image() noexcept = default;
    vk::Image GetHandle() { return image_.GetHandle(); };
    const ImageInfo& GetInfo() { return info_; }

private:
    Image(VmaAllocator allocator, const ImageInfo& info) noexcept;
    Image(Image&& other) noexcept : image_(std::move(other.image_)), info_(other.info_), view_(std::move(other.view_)) {}

private:
    VmaImage image_;
    ImageInfo info_;
    ImageView view_;
};

} // namespace X::Backend
