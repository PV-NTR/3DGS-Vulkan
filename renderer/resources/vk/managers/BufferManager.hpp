#pragma once

#include <memory>
#include <unordered_map>
#include <queue>

#include "resources/vk/Buffer.hpp"

namespace X::Backend {

class BufferManager {
public:
    BufferManager() = default;
    std::shared_ptr<Buffer> RequireBuffer(const BufferInfo& info);

private:
    union BufferKey {
        struct {
            uint64_t size_ : 32;
            uint32_t usage_ : 10;
            uint32_t memProps_ : 9;
            uint32_t padding_ : 11;
        };
        uint64_t packed_;
    };
    static BufferKey GetKeyFromBufferInfo(const BufferInfo& info);
    void Recycle(Buffer* bufferPtr);

private:
    std::unordered_map<uint64_t, std::unique_ptr<Buffer>> freeResources_;
    std::queue<uint64_t> resourceQueue_;
    static inline constexpr uint32_t CACHE_SIZE = 256;
};

} // namespace X::Backend