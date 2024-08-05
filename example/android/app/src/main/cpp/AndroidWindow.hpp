#pragma once

#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "GaussianRenderer.hpp"
#include "scenes/Scene.hpp"
#include "resources/vk/DisplaySurface.hpp"

extern android_app* g_androidAppCtx;

class AndroidWindow {
public:
    AndroidWindow() = default;
    ~AndroidWindow() = default;
    bool Init(android_app* app);
    void RenderLoop();
    void HandleAppCommand(int32_t cmd);

protected:
    void GetDeviceConfig();
    void InitBackend();
    void InitRenderer();
    void InitSurface(ANativeWindow* window);
    void LoadScene();

    void HandleAppInputs();
    void HandleMotionEvent(GameActivityMotionEvent* motionEvent);
    void KeyPressed(uint32_t) {};       // realize this function for other interactions

private:
    std::unique_ptr<X::GaussianRenderer> renderer_ = nullptr;
    std::unique_ptr<X::Scene> scene_ = nullptr;
    std::unique_ptr<X::Backend::DisplaySurface> surface_ = nullptr;

    // State of touch input
    bool focused_ = false;
    struct TouchPos {
        int32_t x_;
        int32_t y_;
    } touchPos_;
    struct {
        glm::vec2 axisLeft_ = glm::vec2(0.0f);
        glm::vec2 axisRight_ = glm::vec2(0.0f);
    } gamePadState_;
    bool touchDown_ = false;
    double touchTime_ = 0.0;
    int64_t lastTapTime_ = 0;

    // TODO: create a time statistics class
    std::chrono::time_point<std::chrono::high_resolution_clock> frameStart_;

    // device config
    int32_t screenDensity_;
};