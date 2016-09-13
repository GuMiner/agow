#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include <deque>
#include "Math\Vec.h"

// Manages a camera that follows the user sans jerkiness.
class Camera
{
    vec::vec3 cameraPos;
    vec::quaternion cameraRotation;

    btRigidBody* playerObject;

    // In degrees.
    vec::vec2 yawLimits;
    vec::vec2 pitchLimits;
    float centerPitch;

    float currentYaw;
    float currentPitch;

    std::deque<vec::mat4> lastMatrices;

public:
    Camera(float centerPitch, vec::vec2 yawLimits, vec::vec2 pitchLimits);
    void Initialize(btRigidBody* playerObject);

    void Update(float elapsedTime);
    void Yaw(float factor);
    void Pitch(float factor);

    const vec::vec3 GetViewPosition() const;
    const vec::quaternion GetViewOrientation() const;
    const vec::mat4 GetViewMatrix() const;
};

