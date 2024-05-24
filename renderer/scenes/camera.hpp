#pragma once

#include "common/glm_common.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace X {

class Camera {
public:
    Camera() = default;
    Camera(const Camera& oth) = default;
    ~Camera() = default;

    enum class CameraType {
        LookAt,
        FirstPerson
    };

    bool Moving();

    float GetNearClip() { return zNear_; }
    float GetFarClip() { return zFar_; }

    void SetPerspective(float fovY, float aspect, float zNear, float zFar);
    void SetPosition(glm::vec3 position);
    void SetRotation(glm::vec3 rotation);
    void SetTranslation(glm::vec3 translation);

    void Rotate(glm::vec3 delta);
    void Translate(glm::vec3 delta);

    void SetRotationSpeed(float rotationSpeed) { this->rotationSpeed_ = rotationSpeed; }
    void SetMovementSpeed(float movementSpeed) { this->movementSpeed_ = movementSpeed; }

    void UpdateAspectRatio(float aspect);

    void Update(float deltaTime);
    bool UpdatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime);

private:
    void UpdateViewMatrix();
    glm::vec3 GetCameraFront();
    bool UpdateCameraFirstPerson(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime, float deadZone);
    bool UpdateCameraLookAt(glm::vec2 axisLeft, glm::vec2 axisRight, float deadZone);

private:
    CameraType type_ = CameraType::LookAt;

    // extrinsic param
    glm::vec3 rotation_ = glm::vec3();
    glm::vec3 position_ = glm::vec3();

    // intrinsic param
    float fovY_;
    float aspect_;
    float zNear_, zFar_;

    // for camera moving
    struct {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keys_;
    float rotationSpeed_ = 1.0f;
    float movementSpeed_ = 1.0f;
    bool updated_ = true;

    // cached MVP
    struct {
        glm::mat4 perspective = glm::mat4();
        glm::mat4 view = glm::mat4();
    } matrices_;
};

} // namespace X