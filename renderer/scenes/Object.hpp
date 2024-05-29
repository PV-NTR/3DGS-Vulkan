#pragma once

#include "common/GLMCommon.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "utils/Utils.hpp"

namespace X {

class Object {
public:

private:
    uint32_t objectID_ = NewID();
    glm::vec3 position_;
    glm::quat rotation_;
    glm::vec3 scale_;
    glm::mat4 transform_;
};

} // namespace X