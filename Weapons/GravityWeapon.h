#pragma once
#include <string>
#include <deque>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include "WeaponBase.h"

class GravityWeapon : public WeaponBase
{
protected:
    virtual float GetRequiredAmmoToFire() override;
    virtual void FireInternal(glm::vec3 fireOrigin, glm::quat fireDirection) override;

public:
    GravityWeapon::GravityWeapon(BasicPhysics* physics);
    void Render(const glm::mat4& projectionMatrix) override;
};

