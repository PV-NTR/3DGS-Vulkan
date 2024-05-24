#pragma once

#include <vector>
#include <memory>

#include "object.hpp"

namespace X {

class Scene {
public:
    Scene() = default;
    Scene(Scene&& other) = default;
    ~Scene() = default;
    void AddObject(Object&& object);

private:
    Scene(const Scene& other) = delete;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

} // namespace X