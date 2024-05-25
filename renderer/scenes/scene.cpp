#include "scene.hpp"

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
bool Scene::SceneChanged()
{
    return camera_.Updated();
}

} // namespace X