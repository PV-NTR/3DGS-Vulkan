#include "ImageView.hpp"
#include "VkContext.hpp"
#include "Image.hpp"

namespace X::Backend {

ImageView::ImageView(std::shared_ptr<Image> image) noexcept
{
    // auto& info = image->GetInfo();
    vk::ImageViewCreateInfo viewCI;
    vk::ImageSubresourceRange subreourceRange;
    subreourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setLevelCount(1)
        .setLayerCount(1);
    if (image->GetInfo().depthStencil_) {
        subreourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
    }
    viewCI.setViewType(vk::ImageViewType::e2D)
        .setImage(image->GetHandle())
        // TODO: format from info, ycbcr or astc or other info
        .setFormat(image->GetInfo().format_)
        .setComponents({ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA })
        .setSubresourceRange(subreourceRange);
    auto [ret, uniqueView] = VkContext::GetInstance().GetDevice().createImageViewUnique(viewCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateImageView failed, errCode: %d", ret);
    }
    uniqueView_.swap(uniqueView);
    view_ = *uniqueView_;
    this->DependOn(image);
}

ImageView::ImageView(ImageView&& other) noexcept
    : uniqueView_(std::move(other.uniqueView_)), view_(std::move(other.view_))
{

}

ImageView& ImageView::operator=(ImageView&& other) noexcept
{
    uniqueView_ = std::move(other.uniqueView_);
    view_ = std::move(other.view_);
    return *this;
}

} // namespace X::Backend