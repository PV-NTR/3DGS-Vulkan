#include "Scene.hpp"

namespace X {

void Scene::AddObject(Object&& object)
{
    objects_.emplace_back(std::make_unique<Object>(object));
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

bool Scene::OverlayVisible()
{
    return overlay_.IsVisible();
}

bool Scene::OverlayChanged() const
{
    return overlay_.Updated();
}

bool Scene::ObjectChanged() const
{
    // TODO: allow change of object
    return false;
}

bool Scene::SceneChanged() const
{
    return camera_.Updated() || OverlayChanged() || ObjectChanged();
}

} // namespace X