#pragma once

#include <vector>
#include <memory>

#include "Camera.hpp"
#include "Object.hpp"
#include "UIOverlay.hpp"

namespace X {

class Scene {
public:
    Scene() = default;
    Scene(Scene&& other) = default;
    virtual ~Scene() = default;
    void AddObject(Object&& object);
    void ChangeOverlayState();
    void ChangeCameraType();
    Camera::CameraType GetCameraType() const { return camera_.GetType(); };
    void UpdateCameraState();
    bool OverlayVisible();
    bool SceneChanged() const;
    bool ObjectChanged() const;
    bool OverlayChanged() const;
    Camera& GetCamera() { return camera_; }

private:
    Scene(const Scene& other) = delete;

private:
    std::vector<std::unique_ptr<Object>> objects_;
    UIOverlay overlay_;
    Camera camera_;
};

} // namespace X