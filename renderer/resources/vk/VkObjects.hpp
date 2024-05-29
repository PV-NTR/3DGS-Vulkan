#pragma once

#include "VkResource.hpp"
#include "vma/VmaBuffer.hpp"
#include "vma/VmaImage.hpp"

namespace X::Backend {

class ImageManager;
class BufferManager;

using VulkanBuffer = VkResource<VmaBuffer, BufferManager>;
using VulkanImage = VkResource<VmaImage, ImageManager>;
    
} // namespace X
