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
    bool OverlayChanged() const;
    Camera& GetCamera() { return camera_; }
    void UpdateData();

private:
    Scene(const Scene& other) = delete;

private:
    friend class Renderer;
    friend class GaussianRenderer;
    std::vector<std::shared_ptr<Object>> objects_;
    UIOverlay overlay_;
    Camera camera_;

    std::bitset<32> objectStatus_;
    std::vector<std::shared_ptr<Backend::Buffer>> ssboIntrinsic_;
    std::vector<std::shared_ptr<Backend::Buffer>> ssboExtrinsic_;
    std::shared_ptr<Backend::Buffer> uboPrefixSums_;
    std::vector<std::shared_ptr<Backend::Buffer>> uboModels_;
    std::shared_ptr<Backend::Buffer> uboCamera_;
    uint32_t totalPointCount_ = 0;
};

} // namespace X