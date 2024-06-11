#pragma once

#include "Renderer.hpp"

namespace X {

class GaussianRenderer : public Renderer {
public:
    GaussianRenderer();

private:
    vk::UniqueRenderPass mainRenderPassUnique_;
    vk::RenderPass mainRenderPass_;
};
    
} // namespace X
