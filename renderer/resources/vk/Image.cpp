#include "Image.hpp"
#include "VkContext.hpp"
#include "ImageView.hpp"

namespace X::Backend {

Image::Image(VmaAllocator allocator, const ImageInfo& info) noexcept
    : image_(allocator, { info.width_, info.height_, vk::Format::eR8G8B8A8Unorm,
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment |
        vk::ImageUsageFlagBits::eInputAttachment }), 
      info_(info), view_(shared_from_this())
{

}

} // namespace X::Backend
