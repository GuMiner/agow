#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include "Math\Vec.h"

// Manages a camera that follows the user sans jerkiness.
class Camera
{
    vec::vec3 cameraPos;
    vec::quaternion cameraRotation;

    btRigidBody* playerObject;

public:
    Camera();
    void Initialize(btRigidBody* playerObject);

    void Update(float elapsedTime);


    const vec::vec3 GetViewPosition() const;
    const vec::quaternion GetViewOrientation() const;
};

