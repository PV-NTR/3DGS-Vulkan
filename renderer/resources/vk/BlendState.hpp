#pragma once

#include <array>
#include "common/VkCommon.hpp"

namespace X::Backend {

class BlendState {
public:
    union {
        struct {
            uint32_t srcColor_ : 5;
            uint32_t dstColor_ : 5;
            uint32_t colorOp_ : 6;
            uint32_t srcAlpha_ : 5;
            uint32_t dstAlpha_ : 5;
            uint32_t alphaOp_ : 6;
        };
        uint32_t packed_;
    };
    vk::BlendOp GetColorBlendOp() const;
    vk::BlendOp GetAlphaBlendOp() const;

private:
    static constexpr uint32_t BLEND_OP_NUM = 51;
    static const std::array<vk::BlendOp, BLEND_OP_NUM> blendOpMap_;
};

} // namespace X::Backend
