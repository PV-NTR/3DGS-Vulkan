#pragma once

#include "VmaObject.hpp"

namespace X::Backend {

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
    VmaImage(VmaAllocator allocator, const VmaImageInfo& info) noexcept;
    ~VmaImage() noexcept;

    VmaImage(VmaImage&& other) noexcept;
    VmaImage& operator=(VmaImage&& other) noexcept;

    void Destroy() noexcept;

private:
    vk::Image handle_;
    VmaImageInfo info_;
};

} // namespace X::Backend
