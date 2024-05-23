#pragma once

#include <vector>
#include <thread>

#include "resources/vkcontext.hpp"
#include "utils/singleton.hpp"

class Renderer : public Singleton<Renderer> {
public:
    Renderer();
    ~Renderer();

    bool IsReady();
    bool Init();
    void Destroy();
    void DrawFrame();

private:
    // TODO: multi thread
    VkContext context_;
    /* data */
};
