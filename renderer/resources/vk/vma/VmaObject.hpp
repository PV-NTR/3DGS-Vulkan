#pragma once

#include "common/VkCommon.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

class VmaObject {
public:
    VmaObject(VmaAllocator allocator, bool coherent = false, bool persistent = false) noexcept
        : allocator_(allocator), coherent_(coherent), persistent_(persistent) {}

    VmaObject(VmaObject&& other) noexcept = default;

    bool Mapped() const
    {
        return mappedData_ != nullptr;
    }

    void* Map()
    {
        assert(allocation_ != VK_NULL_HANDLE);
        if (!persistent_ && !Mapped()) {
            auto ret = static_cast<vk::Result>(vmaMapMemory(allocator_, allocation_, &mappedData_));
            if (ret != vk::Result::eSuccess) {
                XLOGE("vmaMapMemory failed, errCode: %d", ret);
            }
        }
        return mappedData_;
    }

    void Unmap()
    {
        if (persistent_ || !mappedData_) {
            return;
        }
        if (allocation_) {
            vmaUnmapMemory(allocator_, allocation_);
            mappedData_ = nullptr;
        }
    }

    void Flush(vk::DeviceSize offset = 0, vk::DeviceSize size = vk::WholeSize)
    {
        assert(allocation_ != VK_NULL_HANDLE);
        if (!coherent_) {
            auto ret = static_cast<vk::Result>(vmaFlushAllocation(allocator_, allocation_, offset, size));
            if (ret != vk::Result::eSuccess) {
                XLOGE("vmaFlushAllocation failed, errCode: %d", ret);
            }
        }
    }

    void Update(const uint8_t* data, size_t size, size_t offset = 0)
    {
        if (persistent_) {
            std::copy(data, data + size, reinterpret_cast<uint8_t*>(mappedData_) + offset);
            Flush();
        } else {
            Map();
            std::copy(data, data + size, reinterpret_cast<uint8_t*>(mappedData_) + offset);
            Flush();
            Unmap();
        }
    }

    void Update(const void* data, size_t size, size_t offset = 0)
    {
        return Update(reinterpret_cast<const uint8_t*>(data), size, offset);
    }

protected:
    VmaAllocator allocator_ = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;

private:
    void* mappedData_ = nullptr;
    bool coherent_ = false;     // where the buffer is host_coherent
    bool persistent_ = false;   // whether the buffer is persistently mapped or not
};
    
} // namespace X::Backend
