#pragma once

#include "VmaObject.hpp"

namespace X::Backend {

struct BufferInfo {
    uint64_t size_;
    vk::BufferUsageFlags usage_;
    vk::MemoryPropertyFlags memProps_;
};

class VmaBuffer : public VmaObject {
public:
    VmaBuffer(VmaAllocator allocator, const BufferInfo& info) noexcept;
    ~VmaBuffer() noexcept;

private:
    void Destroy() noexcept;

};

} // namespace X::Backend
