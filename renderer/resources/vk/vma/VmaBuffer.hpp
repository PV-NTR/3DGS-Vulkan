#pragma once

#include "VmaObject.hpp"

namespace X::Backend {

struct VmaBufferInfo {
    uint64_t size_;
    vk::BufferUsageFlags usage_;
    vk::MemoryPropertyFlags memProps_;
};

class VmaBuffer : public VmaObject {
public:
    VmaBuffer(VmaAllocator allocator, const VmaBufferInfo& info) noexcept;
    ~VmaBuffer() noexcept;

    VmaBuffer(VmaBuffer&& other) noexcept;
    VmaBuffer& operator=(VmaBuffer&& other) noexcept;

    vk::Buffer GetHandle() { return handle_; }

protected:
    void Destroy() noexcept;

private:
    vk::Buffer handle_;
};

} // namespace X::Backend
