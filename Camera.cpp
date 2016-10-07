#include <algorithm>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include "Math\PhysicsOps.h"
#include "BasicPhysics.h"
#include "Camera.h"

Camera::Camera(float centerPitch, glm::vec2 yawLimits, glm::vec2 pitchLimits)
    : centerPitch(centerPitch), yawLimits(yawLimits), pitchLimits(pitchLimits), currentYaw(0), currentPitch(0)
{
}

void Camera::Initialize(btRigidBody* playerObject)
{
    this->playerObject = playerObject;
    glm::vec3 pos = BasicPhysics::GetBodyPosition(playerObject);
    glm::quat orientation = GetViewOrientation();

    glm::vec3 forwardsVector = PhysicsOps::ForwardsVector(orientation);
    glm::vec3 upVector = PhysicsOps::UpVector(orientation);

    // TODO configurable distance from the player.
    cameraPos = pos - (upVector * 0.1f + forwardsVector * 3.5f);

    glm::quat bodyRotation = BasicPhysics::GetBodyRotation(playerObject);
    cameraRotation = glm::conjugate(bodyRotation) * glm::rotate(glm::quat(), glm::radians(centerPitch), glm::vec3(1, 0, 0));

    lastMatrices.push_back(glm::mat4_cast(GetViewOrientation()) * glm::translate(glm::mat4(), -GetViewPosition()));
}

void Camera::Update(float elapsedTime)
{
    // Yaw and pitch rotate.
    glm::quat bodyRotation = BasicPhysics::GetBodyRotation(playerObject);
    glm::quat expectedRotation = glm::conjugate(bodyRotation)
        * glm::rotate(glm::quat(), glm::radians(centerPitch + currentPitch), glm::vec3(1, 0, 0))
        * glm::rotate(glm::quat(), glm::radians(currentYaw), glm::vec3(0, 1, 0));

    cameraRotation = expectedRotation;

    glm::vec3 pos = BasicPhysics::GetBodyPosition(playerObject);
    glm::quat orientation = GetViewOrientation();

    glm::vec3 forwardsVector = PhysicsOps::ForwardsVector(orientation);
    glm::vec3 upVector = PhysicsOps::UpVector(orientation);

    // TODO configurable distance from the player.
    glm::vec3 newCameraPos = pos - (upVector * 0.1f + forwardsVector * 3.5f);
    newCameraPos.z = std::max(newCameraPos.z, pos.z);

    cameraPos = newCameraPos;
    lastMatrices.push_back(glm::mat4_cast(GetViewOrientation()) * glm::translate(glm::mat4(), -GetViewPosition()));
    if (lastMatrices.size() > 7)
    {
        lastMatrices.pop_front();
    }
}

void Camera::Yaw(float factor)
{
    currentYaw += factor;
    currentYaw = std::min(currentYaw, yawLimits.y);
    currentYaw = std::max(currentYaw, yawLimits.x);
}

void Camera::Pitch(float factor)
{
    currentPitch += factor;
    currentPitch = std::min(currentPitch, pitchLimits.y);
    currentPitch = std::max(currentPitch, pitchLimits.x);
}

const glm::vec3 Camera::GetViewPosition() const
{
    return cameraPos;
}

const glm::quat Camera::GetViewOrientation() const
{
    return cameraRotation;
}

const glm::mat4 Camera::GetViewMatrix() const
{
    return PhysicsOps::Average(lastMatrices);
}