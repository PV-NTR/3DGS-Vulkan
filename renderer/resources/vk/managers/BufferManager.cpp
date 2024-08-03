#include "BufferManager.hpp"

#include "resources/vk/VkContext.hpp"

namespace X::Backend {

BufferManager::BufferKey BufferManager::GetKeyFromBufferInfo(const BufferInfo& info)
{
    auto [usage, prop] = Buffer::GetPropsFromType(info.type_);
    BufferManager::BufferKey key{};
    key.size_ = info.size_;
    key.usage_ = static_cast<uint32_t>(usage);
    key.memProps_ = static_cast<uint32_t>(prop);
    key.padding_ = 0;
    return key;
}

std::shared_ptr<Buffer> BufferManager::RequireBuffer(const BufferInfo& info)
{
    auto key = GetKeyFromBufferInfo(info);
    if (freeResources_.find(key.packed_) == freeResources_.end()) {
        freeResources_[key.packed_]
            = std::unique_ptr<Buffer>(new Buffer(VkContext::GetInstance().GetAllocator(), info));
    }
    std::shared_ptr<Buffer> ret(freeResources_[key.packed_].release(), [this](Buffer* bufferPtr) {
            this->Recycle(bufferPtr);
        });
    assert(ret != nullptr);
    freeResources_.erase(key.packed_);
    return ret;
}
   
void BufferManager::Recycle(Buffer* bufferPtr)
{
    while (freeResources_.size() >= CACHE_SIZE) {
        auto frontFreeResource = resourceQueue_.front();
        resourceQueue_.pop();
        freeResources_.erase(frontFreeResource);
    }
    auto key = GetKeyFromBufferInfo(bufferPtr->info_);
    resourceQueue_.push(key.packed_);
    freeResources_[key.packed_] = std::unique_ptr<Buffer>(new Buffer(std::move(*bufferPtr)));
    delete bufferPtr;
}

} // namespace X::Backend
