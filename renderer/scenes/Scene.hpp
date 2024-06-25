#pragma once

#include <vector>
#include <memory>
#include <bitset>

#include "Camera.hpp"
#include "Object.hpp"
#include "UIOverlay.hpp"

#include "resources/vk/Buffer.hpp"

namespace X {

class Scene {
public:
    Scene();
    Scene(Scene&& other) = default;
    virtual ~Scene() = default;
    void AddObject(std::unique_ptr<Object>&& object);
    void ChangeOverlayState();
    void ChangeCameraType();
    Camera::CameraType GetCameraType() const { return camera_.GetType(); };
    void UpdateCameraState();
    bool OverlayVisible();
    bool ObjectChanged() const;
    bool SceneChanged() const;
    bool OverlayChanged() const;
    Camera& GetCamera() { return camera_; }
    void UpdateData();

private:
    Scene(const Scene& other) = delete;
    void UpdateCameraData();

private:
    friend class Renderer;
    friend class GaussianRenderer;
    std::vector<std::unique_ptr<Object>> objects_;
    UIOverlay overlay_;
    Camera camera_;

    std::bitset<32> objectStatus_ = 0;
    std::vector<std::shared_ptr<Backend::Buffer>> ssboSplatData_;
    std::shared_ptr<Backend::Buffer> uboPrefixSums_;
    std::shared_ptr<Backend::Buffer> uboModels_;
    std::shared_ptr<Backend::Buffer> uboCamera_;
    uint32_t totalPointCount_ = 0;
};

} // namespace X