#include "VmaBuffer.hpp"

namespace X::Backend {

VmaBuffer::VmaBuffer(VmaAllocator allocator, const VmaBufferInfo& info) noexcept
    : VmaObject(allocator, static_cast<bool>(info.memProps_ & vk::MemoryPropertyFlagBits::eHostCoherent))
{
    assert(allocator_ != VK_NULL_HANDLE);
    VkBufferCreateInfo bufferCI {};
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size = info.size_;
    bufferCI.usage = static_cast<VkBufferUsageFlags>(info.usage_);

    VmaAllocationCreateInfo allocationCI {};
    allocationCI.requiredFlags = static_cast<VkMemoryPropertyFlags>(info.memProps_);

    VkBuffer handle;
    auto ret = vmaCreateBuffer(allocator_, &bufferCI, &allocationCI, &handle, &allocation_, &allocationInfo_);
    assert(ret == VK_SUCCESS);
    SetMappedData();
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
