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
    virtual void OnRecordGraphicsCommands(Scene* scene) = 0;
    vk::CommandBuffer GetPresentCmdBuffer() { return presentCmdBuffer_; }
    vk::CommandBuffer GetComputeCmdBuffer() { return computeCmdBuffer_; }
    const std::string GetShaderPath()
    {
#ifdef SHADER_DIR
        return SHADER_DIR;
#else
        return "";
#endif
    }

protected:
    Backend::DisplaySurface* surface_ = nullptr;

private:
    bool ready_ = false;
    bool needCompute_ = false;
    bool commandChanged_ = false;
    bool dataChanged_ = false;

    vk::CommandBuffer presentCmdBuffer_, computeCmdBuffer_;
    // data
    std::shared_ptr<Backend::Buffer> screenSize_;
    bool auxiliaryInited = true;
};

} // namespace X