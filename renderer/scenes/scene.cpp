#include "scene.hpp"

void Scene::AddObject(Object&& object)
{
    objects_.emplace_back(std::make_unique<Object>(object));
}