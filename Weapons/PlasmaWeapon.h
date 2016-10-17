#pragma once
#include <string>
#include <deque>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include "Data\UserPhysics.h"
#include "Managers\ShaderManager.h"
#include "Utils\TypedCallback.h"
#include "WeaponBase.h"

struct PlasmaProjectileData
{
    btRigidBody* body;
    float flightTime;
};

struct PlasmaProgram
{
    GLuint programId;
    GLuint projMatrixLocation;
    GLuint positionLocation;
    GLuint frameTimeLocation;

    GLuint vao;
};

class PlasmaWeapon : public WeaponBase, ICallback<UserPhysics::ObjectType>
{
    static PlasmaProgram program;

protected:
    float maxDistance;

    unsigned int maxProjectiles;
    float lastElapsedTime;

    virtual float GetRequiredAmmoToFire() override;
    virtual void FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection) override;

public:
    PlasmaWeapon::PlasmaWeapon(BasicPhysics* physics);
    static bool LoadGraphics(ShaderManager* shaderManager);
    virtual void Update(float elapsedTime) override;
    void Render(const glm::mat4& projectionMatrix) override;

    virtual void Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData) override;

    static void UnloadGraphics();
};

