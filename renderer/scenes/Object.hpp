#pragma once

#include "common/GLMCommon.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "utils/Utils.hpp"
#include "resources/vk/Buffer.hpp"

namespace X {

class Object {
public:
    enum class Type {
        Mesh = 0,
        Splat,
    };
    explicit Object(Type type) noexcept : type_(type) {}
    Type GetType() const { return type_; }
    void SetPosition(glm::vec3 position)
    {
        position_ = position;
        updated_ = false;
    }
    void SetRotation(glm::quat rotation)
    {
        rotation_ = rotation;
        updated_ = false;
    }
    void SetScale(glm::vec3 scale)
    {
        scale_ = scale;
        updated_ = false;
    }
    glm::mat4 GetTransform()
    {
        if (!updated_) {
            Update();
        }
        return transform_;
    }
    void Update();

private:
    uint32_t objectID_ = NewID();
    Type type_ = Type::Splat;
    glm::vec3 position_ = glm::vec3();
    glm::quat rotation_ = glm::quat();
    glm::vec3 scale_ = glm::vec3(1.0f);
    glm::mat4 transform_ = glm::mat4(1.0f);
    bool updated_ = false;
};

} // namespace X