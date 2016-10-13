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
    rotateAmounts = glm::vec3(0, 1, 0);
    angle = 90;
    
    UpdateCamera();
    lastMatrices.push_back(glm::mat4_cast(GetViewOrientation()) * glm::translate(glm::mat4(), -GetViewPosition()));
}

void Camera::UpdateCamera()
{
    glm::vec3 pos = BasicPhysics::GetBodyPosition(playerObject);

    glm::quat playerOrientation = BasicPhysics::GetBodyRotation(playerObject);
    glm::quat travelRotation = playerOrientation * glm::rotate(glm::quat(), glm::radians(-90.0f), glm::vec3(1, 0, 0));

    glm::vec3 forwardsVector = PhysicsOps::ForwardsVector(travelRotation);
    glm::vec3 upVector = PhysicsOps::UpVector(travelRotation);

    // TODO configurable distance from the player.
    cameraPos = pos - (upVector * 1.0f + forwardsVector * 3.5f);

    cameraRotation = glm::rotate(glm::quat(), glm::radians(centerPitch + currentPitch), glm::vec3(1, 0, 0))
        * glm::rotate(glm::quat(), glm::radians(currentYaw), glm::vec3(0, 0, 1))
        * glm::conjugate(playerOrientation);
}

void Camera::Update(float elapsedTime)
{
    UpdateCamera();

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