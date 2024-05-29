#pragma once

#include "VmaObject.hpp"

namespace X::Backend {

class VmaImage : public VmaObject {
public:
    VmaImage(VmaAllocator allocator) noexcept;
    ~VmaImage() noexcept;

    VmaImage(VmaImage&& other) noexcept;
    VmaImage& operator=(VmaImage&& other) noexcept;

    void reset() noexcept
    {
        Destroy();
    }

private:
    void Destroy() noexcept;

private:
    vk::Image handle_;
};

} // namespace X::Backend
