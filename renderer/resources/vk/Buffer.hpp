#pragma once

#include <unordered_map>

#include "VkResourceBase.hpp"
#include "vma/VmaBuffer.hpp"
#include "resources/Enums.hpp"

namespace X::Backend {

class BufferManager;

struct BufferInfo {
    uint64_t size_;
    BufferType type_;
};

class Buffer : public VkResourceBase {
private:
    friend class BufferManager;
    using BufferProps = std::pair<vk::BufferUsageFlags, vk::MemoryPropertyFlags>;

public:
    virtual ~Buffer() noexcept = default;
    vk::Buffer GetHandle() { return buffer_.GetHandle(); }

private:
    Buffer(VmaAllocator allocator, const BufferInfo& info) noexcept;
    Buffer(Buffer&& other) noexcept : buffer_(std::move(other.buffer_)), info_(other.info_) {}
    static BufferProps GetPropsFromType(BufferType type);

private:
    static std::unordered_map<BufferType, BufferProps> propMap_;
    VmaBuffer buffer_;
    BufferInfo info_;
};

} // namespace X::Backend