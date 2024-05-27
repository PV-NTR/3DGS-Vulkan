#pragma once

#include <vector>
#include <thread>

#include "resources/vk/vkcontext.hpp"
#include "utils/singleton.hpp"
#include "scenes/scene.hpp"

namespace X {

class Renderer : public Singleton<Renderer> {
public:
    Renderer();
    ~Renderer();

    bool IsReady();
    bool Init();
    void Destroy();
    void UpdateScene(const Scene& scene);
    void DrawFrame();

private:
    Backend::VkContext context_;
    bool ready_;
    /* data */
};

} // namespace X