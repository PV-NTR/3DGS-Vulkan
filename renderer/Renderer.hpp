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
    bool AllocateCommandBuffers();
    virtual void RecordGraphicsCommands(Scene* scene);
    virtual void RecordComputeCommands(Scene* scene) = 0;
    virtual void InitAuxiliaryBuffers(Scene* scene) = 0;

    virtual void SubmitGraphicsCommands();

    virtual bool OnInit(Backend::DisplaySurface* surface) = 0;
    virtual void OnUpdateScene(Scene* scene);
    virtual void OnDrawFrame() = 0;
    virtual void OnRecordGraphicsCommands(Scene* scene, std::shared_ptr<Backend::CommandBuffer> cmdBuffer) = 0;
    std::shared_ptr<Backend::CommandBuffer> GetCurrentPresentCmdBuffer() { return presentCmdBuffers_[surface_->GetCurrentFrameIdx()]; }
    std::shared_ptr<Backend::CommandBuffer> GetCurrentComputeCmdBuffer() { return computeCmdBuffers_[surface_->GetCurrentFrameIdx()]; }

protected:
    Backend::DisplaySurface* surface_ = nullptr;
    vk::Fence fence_;
    std::vector<std::shared_ptr<Backend::CommandBuffer>> presentCmdBuffers_, computeCmdBuffers_;

private:
    bool ready_ = false;
    bool needCompute_ = false;
    bool commandChanged_ = false;
    bool dataChanged_ = false;

    // data
    std::shared_ptr<Backend::Buffer> screenSize_;
    // TODO: correct this
    bool auxiliaryInited = true;
};

} // namespace X