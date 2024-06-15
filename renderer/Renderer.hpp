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
    virtual void RecordGraphicsCommands(Scene* scene);
    virtual void RecordComputeCommands(Scene* scene) = 0;

    virtual void SubmitGraphicsCommands();

    virtual bool OnInit(Backend::DisplaySurface* surface);
    virtual void OnUpdateScene(Scene* scene);
    virtual void OnDrawFrame() = 0;
    virtual void OnRecordGraphicsCommands(Scene* scene) = 0;
    vk::CommandBuffer GetCurrentPresentCmdBuffer() { return presentCmdBuffers_[currentFrameIdx_]; }
    vk::CommandBuffer GetCurrentComputeCmdBuffer() { return computeCmdBuffers_[currentFrameIdx_]; }

protected:
    Backend::DisplaySurface* surface_ = nullptr;

private:
    bool ready_ = false;
    bool needCompute_ = false;
    bool commandChanged_ = false;
    bool dataChanged_ = false;

    Backend::CommandPool presentPool_, computePool_;
    std::vector<vk::CommandBuffer> presentCmdBuffers_, computeCmdBuffers_;
    uint32_t currentFrameIdx_ = 0;
    // data
    std::shared_ptr<Backend::Buffer> screenSize_;
};

} // namespace X