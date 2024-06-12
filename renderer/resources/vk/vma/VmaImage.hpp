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

};

class VmaImage : public VmaObject {
public:
    VmaImage() noexcept = default;
    VmaImage(VmaAllocator allocator, const VmaImageInfo& info) noexcept;
    // from external
    explicit VmaImage(vk::Image image) noexcept;
    ~VmaImage() noexcept;

    VmaImage(VmaImage&& other) noexcept;
    VmaImage& operator=(VmaImage&& other) noexcept;
    vk::Image GetHandle() { return handle_; };

protected:
    void Destroy() noexcept;

private:
    vk::Image handle_;
    bool external_ = false;
};

} // namespace X::Backend
