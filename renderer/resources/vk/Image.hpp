#pragma once

#include "VkResourceBase.hpp"
#include "vma/VmaImage.hpp"
#include "resources/Enums.hpp"

namespace X::Backend {

class ImageManager;

struct ImageInfo {
    uint32_t width_;
    uint32_t height_;
    // TODO: support other colortype
    // ColorType colorType_;
};

class Image : public VkResourceBase<Image> {
private:
    friend class ImageManager;
    using Parent = VkResourceBase<Image>;

public:
    virtual ~Image() noexcept = default;

private:
    Image(VmaAllocator allocator, const ImageInfo& info) noexcept;
    Image(Image&& other) noexcept : image_(std::move(other.image_)), info_(other.info_) {}

private:
    VmaImage image_;
    ImageInfo info_;
};

} // namespace X::Backend
