#include "Math\MathOps.h"
#include "BasicPhysics.h"
#include "Camera.h"

Camera::Camera()
{
}

void Camera::Initialize(btRigidBody* playerObject)
{
    this->playerObject = playerObject;
}

void Camera::Update(float elapsedTime)
{
    vec::vec3 pos = BasicPhysics::GetBodyPosition(playerObject);
    vec::quaternion orientation = GetViewOrientation();

    vec::vec3 forwardsVector = orientation.forwardVector();
    vec::vec3 upVector = orientation.upVector();

    // TODO configurable.
    // The view position is behind the player.
    cameraPos =  pos - (upVector * 0.1f + forwardsVector * 3.5f);

    vec::quaternion bodyRotation = BasicPhysics::GetBodyRotation(playerObject);

    // Our camera is rotated 90 degrees to be visible. TODO configurable
    cameraRotation =  bodyRotation.conjugate() * vec::quaternion::fromAxisAngle(MathOps::Radians(80), vec::vec3(1, 0, 0));
}

const vec::vec3 Camera::GetViewPosition() const
{
    return cameraPos;
}

const vec::quaternion Camera::GetViewOrientation() const
{
    return cameraRotation;
}