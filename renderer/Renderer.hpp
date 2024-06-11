#pragma once

#include <vector>
#include <thread>

#include "utils/Singleton.hpp"
#include "scenes/Scene.hpp"
#include "resources/vk/Surface.hpp"

namespace X {

class Renderer {
public:
    Renderer();
    virtual ~Renderer();

    bool IsReady();
    bool Init(Backend::Surface* surface);
    void Destroy();
    void UpdateScene(Scene* scene);
    void DrawFrame();

protected:
    virtual bool OnInit(Backend::Surface* surface);
    virtual void OnUpdateScene(Scene* scene);
    virtual void OnDrawFrame();

private:
    bool ready_ = false;

    Backend::Surface* surface_ = nullptr;

    vk::UniqueCommandPool presentPoolUnique_, computePoolUnique_;
    vk::CommandPool presentPool_, computePool_;
    // data
};

} // namespace X