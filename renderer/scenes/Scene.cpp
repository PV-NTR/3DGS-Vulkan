#include "Scene.hpp"
#include "Splat.hpp"

#include "resources/vk/VkResourceManager.hpp"

namespace X {

void Scene::AddObject(Object&& object)
{
    objects_.emplace_back(std::make_unique<Object>(object));
    if (uboPrefixSums_) {
        uboPrefixSums_ = Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ 32 * 4, BufferType::Uniform });
    }
    Object* obj = objects_.back().get();
    if (obj->GetType() == Object::Type::Splat) {
        Splat* splat = static_cast<Splat*>(obj);
        totalPointCount_ += splat->GetPointCount();
        uboPrefixSums_->Update(&totalPointCount_, 1, objects_.size() - 1);
        // TODO: fix size: need align!
        ssboIntrinsic_.emplace_back(Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ splat->GetPointCount() * 48 * 4, BufferType::Storage}));
        ssboExtrinsic_.emplace_back(Backend::VkResourceManager::GetInstance().GetBufferManager().RequireBuffer({ splat->GetPointCount() * 45 * 4, BufferType::Storage }));
        // TODO: add data from object
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

void Scene::UpdateData()
{
    if (objectStatus_ != 0) {
        for (uint32_t i = 0; i < objects_.size(); i++) {
            if (objectStatus_[i]) {
                glm::mat4 model = objects_[i]->GetTransform();
                uboModels_[i]->Update(&model, 16, 0);
            }
        }
    }

    if (camera_.Updated()) {
        struct {
            glm::mat4 view;
            glm::mat4 proj;
            glm::vec2 focal;
        } data;
        data.view = camera_.matrices_.view;
        data.proj = camera_.matrices_.perspective;
        data.focal[0] = camera_.fovY_ * camera_.aspect_;
        data.focal[1] = camera_.fovY_;
        uboCamera_->Update(&data, sizeof(data), 0);
    }

    if (OverlayChanged()) {

    }
}

bool Scene::OverlayVisible()
{
    return overlay_.IsVisible();
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