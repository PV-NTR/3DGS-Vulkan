#pragma once

#include "Renderer.hpp"
#include "resources/vk/RenderPass.hpp"

namespace X {

class GaussianRenderer : public Renderer {
public:
    GaussianRenderer();

protected:
    void RecordGraphicsCommands() override;
    void RecordComputeCommands() override {};

private:
    std::shared_ptr<Backend::RenderPass> renderPass_;
};
    
} // namespace X
