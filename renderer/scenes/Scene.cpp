#include "Scene.hpp"
#include "Splat.hpp"

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

    Object* obj = objects_.back().get();
    if (obj->GetType() == Object::Type::Splat) {
        Splat* splat = static_cast<Splat*>(obj);
        totalPointCount_ += splat->GetPointCount();
        uboPrefixSums_->Update(&totalPointCount_, 4, 4 * (objects_.size() - 1));
        // TODO: compress data
        ssboSplatData_.emplace_back(Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ splat->GetPointCount() * sizeof(RawGaussianPoint), BufferType::Storage}));
        ssboSplatData_.back()->Init(0);
        ssboSplatData_.back()->Update(splat->GetPointData(), splat->GetPointCount() * sizeof(RawGaussianPoint), 0);
    }
    objectStatus_.set(objects_.size() - 1, true);
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