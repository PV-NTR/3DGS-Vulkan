#include "VmaBuffer.hpp"

namespace X::Backend {

VmaBuffer::VmaBuffer(VmaAllocator allocator, const VmaBufferInfo& info) noexcept
    : VmaObject(allocator, static_cast<bool>(info.memProps_ & vk::MemoryPropertyFlagBits::eHostCoherent))
{
    assert(allocator_ != VK_NULL_HANDLE);
    VkBufferCreateInfo bufferCI {};
    bufferCI.size = info.size_;
    bufferCI.usage = static_cast<VkBufferUsageFlags>(info.usage_);

    VmaAllocationCreateInfo allocationCI {};
    allocationCI.memoryTypeBits = static_cast<VkImageUsageFlags>(info.memProps_);

    VkBuffer handle;
    auto ret = vmaCreateBuffer(allocator_, &bufferCI, &allocationCI, &handle, &allocation_, nullptr);
    handle_ = handle;
}

VmaBuffer::~VmaBuffer() noexcept
{
    Destroy();
}

VmaBuffer::VmaBuffer(VmaBuffer&& other) noexcept
    : VmaObject(std::move(other)), handle_(other.handle_)
{
    other.handle_ = nullptr;
}

VmaBuffer& VmaBuffer::operator=(VmaBuffer&& other) noexcept
{
    if (this == std::addressof(other)) {
        return *this;
    }
    Destroy();
    new (this) VmaBuffer { std::move(other) };
    return *this;
}

void VmaBuffer::Destroy() noexcept
{
    assert(allocator_ != VK_NULL_HANDLE);
    if (allocation_ != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator_, handle_, allocation_);
    }
}

} // namespace X::Backend
