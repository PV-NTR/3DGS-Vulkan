#include "scene.hpp"

namespace X {

void Scene::AddObject(Object&& object)
{
    objects_.emplace_back(std::make_unique<Object>(object));
}

} // namespace X