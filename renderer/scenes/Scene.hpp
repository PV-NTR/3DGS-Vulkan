#pragma once

#include <vector>
#include <memory>
#include <bitset>

#include "Camera.hpp"
#include "Object.hpp"
#include "UIOverlay.hpp"

#include "resources/vk/DisplaySurface.hpp"
#include "resources/vk/Buffer.hpp"

namespace X {

class Scene {
public:
    Scene();
    Scene(Scene&& other) = default;
    virtual ~Scene() = default;
    void AddObject(std::unique_ptr<Object>&& object);
    void InitGPUData();

    void ChangeOverlayState();
    void ChangeCameraType();
    Camera::CameraType GetCameraType() const { return camera_.GetType(); };
    void UpdateCameraState();
    bool OverlayVisible();
    bool ObjectChanged() const;
    bool SceneChanged() const;
    bool OverlayChanged() const;
    Camera& GetCamera() { return camera_; }
    void UpdateData(Backend::DisplaySurface* surface);

private:
    friend class Renderer;
    friend class GaussianRenderer;
    Scene(const Scene& other) = delete;
    void UpdateCameraData(Backend::DisplaySurface* surface);
    void SortSplatsByDepth();
    void RadixSortSplatsByDepth();
    float GetDepth(uint32_t index);

private:
    std::vector<std::unique_ptr<Object>> objects_;
    UIOverlay overlay_;
    Camera camera_;

    std::bitset<32> objectStatus_ = 0;
    std::vector<uint32_t> sortedSplatIndices_;
    std::vector<uint32_t> prefixSums_;
    std::shared_ptr<Backend::Buffer> ssboSplatData_;
    std::shared_ptr<Backend::Buffer> ssboSortedSplats_;
    std::shared_ptr<Backend::Buffer> uboPrefixSums_;
    std::shared_ptr<Backend::Buffer> uboModels_;
    std::shared_ptr<Backend::Buffer> uboCamera_;
    uint32_t totalPointCount_ = 0;
};

} // namespace X