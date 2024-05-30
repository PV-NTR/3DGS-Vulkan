#include "ImageManager.hpp"

#include "resources/vk/VkContext.hpp"

namespace X::Backend {

union ImageKey {
    struct {
        uint32_t width_ : 14;
        uint32_t height_ : 14;
        uint32_t format_ : 10;
        uint32_t usage_ : 10;
        uint32_t memProps_ : 9;
        uint32_t padding_ : 7;
    };
    uint64_t packed_;
};

static ImageKey GetKeyFromImageInfo(const ImageInfo& info)
{
    ImageKey key {};
    key.width_ = info.width_;
    key.height_ = info.height_;
    key.format_ = static_cast<uint32_t>(vk::Format::eR8G8B8A8Unorm);
    key.usage_ = static_cast<uint32_t>(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment |
        vk::ImageUsageFlagBits::eInputAttachment);
    key.memProps_ = static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eDeviceLocal);
    key.padding_ = 0;
    return key;
}

std::shared_ptr<Image> ImageManager::RequireImage(const ImageInfo& info)
{
    auto key = GetKeyFromImageInfo(info);
    if (freeResources_.find(key.packed_) == freeResources_.end()) {
        freeResources_[key.packed_] = std::unique_ptr<Image>(new Image(VkContext::GetInstance().GetAllocator(), info));
    }
    std::shared_ptr<Image> ret(freeResources_[key.packed_].release(), [this](Image* imagePtr) {
            this->Recycle(imagePtr);
        });
    assert(ret != nullptr);
    freeResources_.erase(key.packed_);
    return ret;
}
   
void ImageManager::Recycle(Image* imagePtr)
{
    while (freeResources_.size() >= CACHE_SIZE) {
        auto frontFreeResource = resourceQueue_.front();
        resourceQueue_.pop();
        freeResources_.erase(frontFreeResource);
    }
    auto key = GetKeyFromImageInfo(imagePtr->info_);
    resourceQueue_.push(key.packed_);
    freeResources_[key.packed_] = std::unique_ptr<Image>(new Image(std::move(*imagePtr)));
    delete imagePtr;
}

} // namespace X::Backend
