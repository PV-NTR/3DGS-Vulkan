#include "Image.hpp"
#include "VkContext.hpp"
#include "ImageView.hpp"

namespace X::Backend {

Image::Image(VmaAllocator allocator, const ImageInfo& info) noexcept
    : info_(info)
{
    vk::ImageUsageFlags flag = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eInputAttachment;
    if (info.depthStencil_) {
        flag |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    } else {
        flag |= vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
    }
    image_ = VmaImage(allocator, { info.width_, info.height_, info.format_, flag, vk::MemoryPropertyFlagBits::eDeviceLocal });
    view_ = ImageView(shared_from_this());
}

Image::Image(vk::Image image, const ImageInfo& info) noexcept
    : image_(image), info_(info), view_(shared_from_this())
{

}

} // namespace X::Backend
