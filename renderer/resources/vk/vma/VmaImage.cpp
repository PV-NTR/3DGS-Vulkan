#include "VmaImage.hpp"

namespace X::Backend {

VmaImage::VmaImage(VmaAllocator allocator) noexcept
    : VmaObject(allocator)
{

}

VmaImage::~VmaImage() noexcept
{

}

VmaImage::VmaImage(VmaImage&& other) noexcept
    : VmaObject(std::move(other)), handle_(std::exchange(other.handle_, VK_NULL_HANDLE))
{
    other.handle_ = VK_NULL_HANDLE;
}

VmaImage& VmaImage::operator=(VmaImage&& other) noexcept
{
    if (this == std::addressof(other)) {
        return *this;
    }
    Destroy();
    new (this) VmaImage{ std::move(other) };
    return *this;
}

void VmaImage::Destroy() noexcept
{

}
    
} // namespace X::Backend
