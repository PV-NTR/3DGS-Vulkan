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
    void SetPosition(glm::vec3 position) { position_ = position; }
    void SetRotation(glm::quat rotation) { rotation_ = rotation; }
    void SetScale(glm::vec3 scale) { scale_ = scale; }
    glm::mat4 GetTransform()
    {
        Update();
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
};

} // namespace X