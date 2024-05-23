#include "camera.hpp"

bool Camera::Moving()
{
    return keys_.left || keys_.right || keys_.up || keys_.down;
}

void Camera::SetPerspective(float fovY, float aspect, float zNear, float zFar)
{
    glm::mat4 currentMatrix = matrices_.perspective;
    this->fovY_ = fovY;
    this->aspect_ = aspect;
    this->zNear_ = zNear;
    this->zFar_ = zFar;
    matrices_.perspective = glm::perspective(glm::radians(fovY), aspect, zNear, zFar);
    if (matrices_.view != currentMatrix) {
        updated_ = true;
    }
};

void Camera::SetPosition(glm::vec3 position)
{
    this->position_ = position;
    UpdateViewMatrix();
}

void Camera::SetRotation(glm::vec3 rotation)
{
    this->rotation_ = rotation;
    UpdateViewMatrix();
}

void Camera::SetTranslation(glm::vec3 translation)
{
    this->position_ = translation;
    UpdateViewMatrix();
};

void Camera::Rotate(glm::vec3 delta)
{
    this->rotation_ += delta;
    UpdateViewMatrix();
}

void Camera::Translate(glm::vec3 delta)
{
    this->position_ += delta;
    UpdateViewMatrix();
}

void Camera::UpdateAspectRatio(float aspect)
{
    this->aspect_ = aspect;
    glm::mat4 currentMatrix = matrices_.perspective;
    matrices_.perspective = glm::perspective(glm::radians(fovY_), aspect, zNear_, zFar_);
    if (matrices_.view != currentMatrix) {
        updated_ = true;
    }
}

glm::vec3 Camera::GetCameraFront()
{
    glm::vec3 camFront;
    camFront.x = -cos(glm::radians(rotation_.x)) * sin(glm::radians(rotation_.y));
    camFront.y = sin(glm::radians(rotation_.x));
    camFront.z = cos(glm::radians(rotation_.x)) * cos(glm::radians(rotation_.y));
    return glm::normalize(camFront);
}

void Camera::Update(float deltaTime)
{
    updated_ = false;
    if (type_ == CameraType::FirstPerson) {
        if (Moving()) {
            glm::vec3 camFront = GetCameraFront();

            float moveSpeed = deltaTime * movementSpeed_;

            if (keys_.up)
                position_ += camFront * moveSpeed;
            if (keys_.down)
                position_ -= camFront * moveSpeed;
            if (keys_.left)
                position_ -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            if (keys_.right)
                position_ += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
        }
    }
    UpdateViewMatrix();
};

bool Camera::UpdateCameraFirstPerson(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime, float deadZone)
{
    // Use the common console thumbstick layout		
    // Left = view, right = move
    bool ret = false;
    const float range = 1.0f - deadZone;

    glm::vec3 camFront = GetCameraFront();

    float moveSpeed = deltaTime * movementSpeed_ * 2.0f;
    float rotSpeed = deltaTime * rotationSpeed_ * 50.0f;

    // Move
    if (fabsf(axisLeft.y) > deadZone) {
        float pos = (fabsf(axisLeft.y) - deadZone) / range;
        position_ -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
        ret = true;
    }
    if (fabsf(axisLeft.x) > deadZone) {
        float pos = (fabsf(axisLeft.x) - deadZone) / range;
        position_ += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos *
            ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
        ret = true;
    }

    // Rotate
    if (fabsf(axisRight.x) > deadZone) {
        float pos = (fabsf(axisRight.x) - deadZone) / range;
        rotation_.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
        ret = true;
    }
    if (fabsf(axisRight.y) > deadZone) {
        float pos = (fabsf(axisRight.y) - deadZone) / range;
        rotation_.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
        ret = true;
    }
    return ret;
}

bool Camera::UpdateCameraLookAt(glm::vec2 axisLeft, glm::vec2 axisRight, float deadZone)
{
    bool ret;
    if (std::abs(axisLeft.x) > deadZone) {
        rotation_ += glm::vec3(0.0f, axisLeft.x * 0.5f, 0.0f);
        ret = true;
    }
    if (std::abs(axisLeft.y) > deadZone) {
        rotation_ += glm::vec3(axisLeft.y * 0.5f, 0.0f, 0.0f);
        ret = true;
    }

    // Zoom
    if (std::abs(axisRight.y) > deadZone) {
        position_ += glm::vec3(0.0f, 0.0f, axisRight.y * 0.01f);
        ret = true;
    }
    return ret;
}

bool Camera::UpdatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
{
    const float deadZone = 0.0015f;
    bool ret = type_ == CameraType::FirstPerson ? UpdateCameraFirstPerson(axisLeft, axisRight, deltaTime, deadZone) :
        UpdateCameraLookAt(axisLeft, axisRight, deadZone);
    if (ret) {
        UpdateViewMatrix();
    }
    return ret;
}

void Camera::UpdateViewMatrix()
{
    glm::mat4 currentMatrix = matrices_.view;

    glm::mat4 rotM = glm::mat4(1.0f);
    glm::mat4 transM;

    rotM = glm::rotate(rotM, glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotM = glm::rotate(rotM, glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotM = glm::rotate(rotM, glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 translation = position_;
    transM = glm::translate(glm::mat4(1.0f), translation);

    if (type_ == CameraType::FirstPerson) {
        matrices_.view = rotM * transM;
    } else {
        matrices_.view = transM * rotM;
    }

    if (matrices_.view != currentMatrix) {
        updated_ = true;
    }
};