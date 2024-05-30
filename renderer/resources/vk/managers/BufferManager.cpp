#include "BufferManager.hpp"

namespace X::Backend {

union BufferKey {
    struct {
        uint64_t size_ : 32;
        uint32_t usage_ : 10;
        uint32_t memProps_ : 9;
        uint32_t padding_ : 11;
    };
    uint64_t packed_;
};

using BufferProps = std::pair<vk::BufferUsageFlags, vk::MemoryPropertyFlags>;

static std::unordered_map<BufferType, BufferProps> typePropMap = {
    { BufferType::Vertex, { vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Index, { vk::BufferUsageFlagBits::eIndexBuffer| vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Uniform, { vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Storage, { vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Staging, { vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent }},
};

static BufferProps GetBufferPropsFromType(BufferType type)
{
    return typePropMap[type];
}

static BufferKey GetKeyFromBufferInfo(const BufferInfo& info)
{
    auto [usage, prop] = GetBufferPropsFromType(info.type_);
    BufferKey key{};
    key.size_ = info.size_;
    key.usage_ = static_cast<uint32_t>(usage);
    key.memProps_ = static_cast<uint32_t>(prop);
    key.padding_ = 0;
    return key;
}
    
} // namespace X::Backend
