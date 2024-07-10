#include "Scene.hpp"
#include "Splat.hpp"

#include <numeric>

#include "resources/vk/VkResourceManager.hpp"

namespace X {

struct CameraData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec2 focal;
};

Scene::Scene()
{
    uboCamera_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ sizeof(CameraData), BufferType::Uniform });
    uboCamera_->Init(0);
    // UpdateCameraData();
    uboPrefixSums_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ 32 * 4, BufferType::Uniform });
    uboPrefixSums_->Init(0);
    uboModels_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ 32 * sizeof(glm::mat4), BufferType::Uniform });
    uboModels_->Init(0);
}

void Scene::AddObject(std::unique_ptr<Object>&& object)
{
    objects_.emplace_back(std::move(object));
    objectStatus_.set(objects_.size() - 1, true);
}

void Scene::InitGPUData()
{
    prefixSums_.reserve(objects_.size());
    for (auto& uniqueObj : objects_) {
        auto obj = uniqueObj.get();
        if (obj->GetType() == Object::Type::Splat) {
            Splat* splat = static_cast<Splat*>(obj);
            totalPointCount_ += splat->GetPointCount();
            prefixSums_.emplace_back(totalPointCount_);
        }
    }
    uboPrefixSums_->Update(prefixSums_.data(), prefixSums_.size() * sizeof(uint32_t), 0);

    // TODO: compress data
    uint32_t totalPointCount = 0;
    ssboSplatData_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ totalPointCount_ * sizeof(RawGaussianPoint), BufferType::Storage});
    for (auto& uniqueObj : objects_) {
        auto obj = uniqueObj.get();
        if (obj->GetType() == Object::Type::Splat) {
            Splat* splat = static_cast<Splat*>(obj);
            ssboSplatData_->Update(splat->GetPointData(), splat->GetPointCount() * sizeof(RawGaussianPoint), totalPointCount);
            totalPointCount += splat->GetPointCount();
        }
    }
    sortedSplatIndices_.resize(totalPointCount_);
    std::iota(sortedSplatIndices_.begin(), sortedSplatIndices_.end(), 0);
    ssboSortedSplats_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ totalPointCount_ * sizeof(uint32_t), BufferType::Storage});
}

void Scene::ChangeOverlayState()
{

}

void Scene::ChangeCameraType()
{
    camera_.SwitchType();
}

void Scene::UpdateCameraState()
{

}

void Scene::UpdateCameraData(Backend::DisplaySurface* surface)
{
    CameraData data;
    float tanFovY = 2.0f * glm::tan(glm::radians(camera_.fovY_ * 0.5f));
    data.focal = glm::vec2(surface->GetWidth() / (camera_.aspect_ * tanFovY), surface->GetHeight() / tanFovY);
    data.view = camera_.matrices_.view;
    data.proj = camera_.matrices_.perspective;
    uboCamera_->Update((void*)(&data), sizeof(CameraData), 0);
}

float Scene::GetDepth(uint32_t index)
{
    assert(index < totalPointCount_);
    uint32_t objectId = std::upper_bound(prefixSums_.begin(), prefixSums_.end(), index) - prefixSums_.begin();
    // if non-splat objects exist, upper_bound of prefixSum_ garanetees that current objectId is a splat
    Splat* splat = static_cast<Splat*>(objects_[objectId].get());
    if (objectId != 0) {
        index -= prefixSums_[objectId - 1];
    }
    auto gaussianData = splat->GetPointData(index);
    glm::vec4 center(gaussianData.pos[0], gaussianData.pos[1], gaussianData.pos[2], 1.0f);
    glm::vec4 pos2d = camera_.matrices_.perspective * camera_.matrices_.view * splat->GetTransform() * center;
    // TODO: divider zero check
    float depth = pos2d.z / pos2d.w;
    return depth;
}

void Scene::SortSplatsByDepth()
{
    std::sort(sortedSplatIndices_.begin(), sortedSplatIndices_.end(), [this](uint32_t i, uint32_t j) {
        // return i < j;
        return GetDepth(i) < GetDepth(j);
    });
    ssboSortedSplats_->Update(sortedSplatIndices_.data(), sortedSplatIndices_.size() * sizeof(uint32_t), 0);
}

void Scene::UpdateData(Backend::DisplaySurface* surface)
{
    if (objectStatus_ != 0) {
        for (uint32_t i = 0; i < objects_.size(); i++) {
            if (objectStatus_[i]) {
                glm::mat4 model = objects_[i]->GetTransform();
                uboModels_->Update(&model, 16 * 4, i * 16 * 4);
                objectStatus_[i] = 0;
            }
        }
    }

    if (camera_.Updated() || surface->Resized()) {
        UpdateCameraData(surface);
        SortSplatsByDepth();
    }

    if (OverlayChanged()) {

    }
}

bool Scene::OverlayVisible()
{
    return overlay_.IsVisible();
}

bool Scene::ObjectChanged() const
{
    return objectStatus_ != 0;
}

bool Scene::OverlayChanged() const
{
    return overlay_.Updated();
}

bool Scene::SceneChanged() const
{
    return (objectStatus_ != 0) || camera_.Updated() || OverlayChanged();
}

} // namespace X