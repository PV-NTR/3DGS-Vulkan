#pragma once

#include <vector>
#include <thread>

#include "utils/Singleton.hpp"
#include "scenes/Scene.hpp"

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
    bool ready_;
    /* data */
};

} // namespace X