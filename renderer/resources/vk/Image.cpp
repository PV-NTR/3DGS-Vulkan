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
    image_ = VmaImage(allocator, { info.width_, info.height_, info.format_, flag });
}

Image::Image(vk::Image image, const ImageInfo& info, VmaImageState&& state) noexcept
    : image_(image, std::move(state)), info_(info)
{
}

void Image::CreateView()
{
    view_ = std::make_shared<ImageView>(shared_from_this());
}

} // namespace X::Backend
