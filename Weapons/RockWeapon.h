#pragma once
#include <string>
#include <deque>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include "Managers\ModelManager.h"
#include "WeaponBase.h"

class RockWeapon : public WeaponBase
{
public:
    enum SizeSetting
    {
        BULLET,
        BOULDER,
        BUILDING
    };

protected:
    ModelManager* modelManager;

    SizeSetting sizeSetting;
    
    glm::vec2 speedLimits;
    float speed;

    int maxProjectiles;

    virtual float GetRequiredAmmoToFire() override;
    virtual void FireInternal(glm::vec3 fireOrigin, glm::quat fireDirection) override;

public:
    RockWeapon::RockWeapon(ModelManager* modelManager, BasicPhysics* physics, glm::vec2 speedLimits);
    void Render(const glm::mat4& projectionMatrix) override;
};

