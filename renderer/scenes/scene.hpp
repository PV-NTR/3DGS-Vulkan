#pragma once

#include <vector>
#include <memory>

#include "camera.hpp"
#include "object.hpp"
#include "uioverlay.hpp"

namespace X {

class Scene {
public:
    Scene() = default;
    Scene(Scene&& other) = default;
    ~Scene() = default;
    void AddObject(Object&& object);
    void ChangeOverlayState();
    void ChangeCameraType();
    Camera::CameraType GetCameraType() const { return camera_.GetType(); };
    void UpdateCameraState();
    bool OverlayVisible();
    bool SceneChanged();
    Camera& GetCamera() { return camera_; }

private:
    Scene(const Scene& other) = delete;

private:
    std::vector<std::unique_ptr<Object>> objects_;
    UIOverlay overlay_;
    Camera camera_;
};

} // namespace X