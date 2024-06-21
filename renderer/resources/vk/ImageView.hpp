#pragma once

#include "common/VkCommon.hpp"
#include "VkResourceBase.hpp"

namespace X::Backend {

class Image;

class ImageView : public VkResourceBase {
public:
    ImageView() noexcept = default;
    explicit ImageView(std::shared_ptr<Image> image) noexcept;
    virtual ~ImageView() = default;
    ImageView(ImageView&& other) noexcept;
    ImageView& operator=(ImageView&& other) noexcept;
    vk::ImageView GetHandle() { return view_; }

private:
    vk::UniqueImageView uniqueView_;
    vk::ImageView view_;
};
    
} // namespace X::Backend
