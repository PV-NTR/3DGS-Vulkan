#include "VmaImage.hpp"

namespace X::Backend {

VmaImage::VmaImage(VmaAllocator allocator, const VmaImageInfo& info) noexcept
    : VmaObject(allocator, static_cast<bool>(info.memProps_ & vk::MemoryPropertyFlagBits::eHostCoherent))
{
    assert(allocator_ != VK_NULL_HANDLE);
    VkImageCreateInfo imageCI {};
    imageCI.imageType = VkImageType::VK_IMAGE_TYPE_2D;
    imageCI.format = static_cast<VkFormat>(info.format_);
    imageCI.extent = VkExtent3D { info.width_, info.height_, 1 };
    imageCI.usage = static_cast<VkImageUsageFlags>(info.usage_);

    VmaAllocationCreateInfo allocationCI {};
    allocationCI.requiredFlags = static_cast<VkMemoryPropertyFlags>(info.memProps_);

    VkImage handle;
    auto ret = vmaCreateImage(allocator_, &imageCI, &allocationCI, &handle, &allocation_, &allocationInfo_);
    SetMappedData();
    handle_ = handle;
}

VmaImage::VmaImage(vk::Image image, VmaImageState&& state) noexcept
    : VmaObject(nullptr), handle_(image), state_(std::move(state)), external_(true)
{

}

VmaImage::~VmaImage() noexcept
{
    Destroy();
}

VmaImage::VmaImage(VmaImage&& other) noexcept
    : VmaObject(std::move(other)), handle_(other.handle_)
{
    other.handle_ = nullptr;
}

VmaImage& VmaImage::operator=(VmaImage&& other) noexcept
{
    if (this == std::addressof(other)) {
        return *this;
    }
    Destroy();
    new (this) VmaImage { std::move(other) };
    return *this;
}

void VmaImage::Destroy() noexcept
{
    assert(external_ || allocator_ != VK_NULL_HANDLE);
    if (allocation_ != VK_NULL_HANDLE) {
        vmaDestroyImage(allocator_, handle_, allocation_);
    }
}
    
} // namespace X::Backend
