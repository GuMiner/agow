#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include <deque>
#include <glm\vec3.hpp>

// Manages a camera that follows the user sans jerkiness.
class Camera
{
    glm::vec3 cameraPos;
    glm::quat cameraRotation;

    btRigidBody* playerObject;

    // In degrees.
    glm::vec2 yawLimits;
    glm::vec2 pitchLimits;
    float centerPitch;

    float currentYaw;
    float currentPitch;

    std::deque<glm::mat4> lastMatrices;

    void UpdateCamera();

    glm::vec3 rotateAmounts;
    float angle;

public:
    Camera(float centerPitch, glm::vec2 yawLimits, glm::vec2 pitchLimits);
    void Initialize(btRigidBody* playerObject);

    void Update(float elapsedTime);
    void Yaw(float factor);
    void Pitch(float factor);

    const glm::vec3 GetViewPosition() const;
    const glm::quat GetViewOrientation() const;
    const glm::mat4 GetViewMatrix() const;
};

