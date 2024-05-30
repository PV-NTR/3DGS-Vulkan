#pragma once

#include <unordered_map>
#include <queue>

#include "resources/vk/Image.hpp"

namespace X::Backend {

class ImageManager {
public:
    ImageManager() = default;
    std::shared_ptr<Image> RequireImage(const ImageInfo& image);

private:
    void Recycle(Image* imagePtr);

private:
    std::unordered_map<uint64_t, std::unique_ptr<Image>> freeResources_;
    std::queue<uint64_t> resourceQueue_;
    static inline constexpr uint32_t CACHE_SIZE = 256;
};

} // namespace X::Backend