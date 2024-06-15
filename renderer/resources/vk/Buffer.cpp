#include "Buffer.hpp"

namespace X::Backend {

std::unordered_map<BufferType, Buffer::BufferProps> Buffer::propMap_ = {
    { BufferType::Vertex, { vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Index, { vk::BufferUsageFlagBits::eIndexBuffer| vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Uniform, { vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Storage, { vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal }},
    { BufferType::Staging, { vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent }},
};

Buffer::BufferProps Buffer::GetPropsFromType(BufferType type)
{
    return propMap_[type];
}

Buffer::Buffer(VmaAllocator allocator, const BufferInfo& info) noexcept
    : buffer_(allocator, { info.size_, propMap_[info.type_].first, propMap_[info.type_].second })
{

}

void Buffer::Update(const void* data, size_t size, size_t offset)
{
    buffer_.Update(data, size, offset);
}

} // namespacae X::Backend