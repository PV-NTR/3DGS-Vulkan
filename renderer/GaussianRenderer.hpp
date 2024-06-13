#pragma once

#include "Renderer.hpp"
#include "resources/vk/RenderPass.hpp"

namespace X {

class GaussianRenderer : public Renderer {
public:
    GaussianRenderer();

protected:
    void RecordComputeCommands(Scene* scene) override;

    void OnRecordGraphicsCommands(Scene* scene) override;
    void OnDrawFrame() override;

private:
};
    
} // namespace X
