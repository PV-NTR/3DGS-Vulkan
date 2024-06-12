#pragma once

#include <vector>
#include <thread>

#include "utils/Singleton.hpp"
#include "scenes/Scene.hpp"
#include "resources/vk/DisplaySurface.hpp"
#include "resources/vk/VkResource.hpp"

namespace X {

class Renderer {
public:
    Renderer(bool needCompute = false);
    virtual ~Renderer();

    bool IsReady();
    bool Init(Backend::DisplaySurface* surface);
    void Destroy();
    void UpdateScene(Scene* scene);
    void DrawFrame();

protected:
    virtual void RecordGraphicsCommands() = 0;
    virtual void RecordComputeCommands() = 0;
    void SubmitGraphicsCommands();
    virtual bool OnInit(Backend::DisplaySurface* surface);
    virtual void OnUpdateScene(Scene* scene);
    virtual void OnDrawFrame();
    vk::CommandBuffer GetCurrentPresentCmdBuffer() { return presentCmdBuffers_[currentFrameIdx_]; }

protected:
    Backend::DisplaySurface* surface_ = nullptr;

private:
    bool ready_ = false;
    bool needCompute_ = false;

    Backend::CommandPool presentPool_, computePool_;
    std::vector<vk::CommandBuffer> presentCmdBuffers_, computeCmdBuffers_;
    uint32_t currentFrameIdx_ = 0;
    std::shared_ptr<Backend::Image> depthStencil_;
    // data
};

} // namespace X