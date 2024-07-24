#include "Object.hpp"

namespace X {

void Object::Update()
{
    transform_ = glm::scale(scale_) * glm::mat4_cast(rotation_);
    transform_[3] = glm::vec4(position_, 1.0f);
    updated_ = true;
}

} // namespace X
