#include <algorithm>
#include "Math\MathOps.h"
#include "Math\VecOps.h"
#include "BasicPhysics.h"
#include "Camera.h"

Camera::Camera(float centerPitch, vec::vec2 yawLimits, vec::vec2 pitchLimits)
    : centerPitch(centerPitch), yawLimits(yawLimits), pitchLimits(pitchLimits), currentYaw(0), currentPitch(0)
{
}

void Camera::Initialize(btRigidBody* playerObject)
{
    this->playerObject = playerObject;
    vec::vec3 pos = BasicPhysics::GetBodyPosition(playerObject);
    vec::quaternion orientation = GetViewOrientation();

    vec::vec3 forwardsVector = orientation.forwardVector();
    vec::vec3 upVector = orientation.upVector();

    // TODO configurable distance from the player.
    cameraPos = pos - (upVector * 0.1f + forwardsVector * 3.5f);

    vec::quaternion bodyRotation = BasicPhysics::GetBodyRotation(playerObject);
    cameraRotation = bodyRotation.conjugate() * vec::quaternion::fromAxisAngle(MathOps::Radians(centerPitch), vec::vec3(1, 0, 0));
}

void Camera::Update(float elapsedTime)
{
    // Yaw and pitch rotate.
    vec::quaternion bodyRotation = BasicPhysics::GetBodyRotation(playerObject);
    vec::quaternion expectedRotation = bodyRotation.conjugate()
        * vec::quaternion::fromAxisAngle(MathOps::Radians(centerPitch + currentPitch), vec::vec3(1, 0, 0))
        * vec::quaternion::fromAxisAngle(MathOps::Radians(currentYaw), vec::vec3(0, 1, 0));

    cameraRotation = expectedRotation;

    this->playerObject = playerObject;
    vec::vec3 pos = BasicPhysics::GetBodyPosition(playerObject);
    vec::quaternion orientation = GetViewOrientation();

    vec::vec3 forwardsVector = orientation.forwardVector();
    vec::vec3 upVector = orientation.upVector();

    // TODO configurable distance from the player.
    vec::vec3 newCameraPos = pos - (upVector * 0.1f + forwardsVector * 3.5f);
    newCameraPos.z = std::max(newCameraPos.z, pos.z);

    vec::vec3 difference = newCameraPos - cameraPos;
    cameraPos += difference * 0.20f; // TODO configurable.
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

const vec::vec3 Camera::GetViewPosition() const
{
    return cameraPos;
}

const vec::quaternion Camera::GetViewOrientation() const
{
    return cameraRotation;  
}