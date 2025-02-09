#pragma once

#include <memory>
#include <unordered_map>
#include <queue>

#include "resources/vk/Image.hpp"

namespace X::Backend {

class ImageManager {
public:
    ImageManager() = default;
    std::shared_ptr<Image> RequireImage(const ImageInfo& info);

private:
    union ImageKey {
        struct {
            uint32_t width_ : 14;
            uint32_t height_ : 14;
            uint32_t format_ : 10;
            uint32_t usage_ : 10;
            uint32_t padding_ : 16;
        };
        uint64_t packed_;
    };
    static ImageKey GetKeyFromImageInfo(const ImageInfo& info);
    void Recycle(Image* imagePtr);

private:
    static inline constexpr uint32_t CACHE_SIZE = 256;

    std::unordered_map<uint64_t, std::unique_ptr<Image>> freeResources_;
    std::queue<uint64_t> resourceQueue_;
};

} // namespace X::Backend