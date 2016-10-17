#pragma once
#include <string>
#include <deque>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include "WeaponBase.h"

class PressureWeapon : public WeaponBase
{
protected:
    float scale;
    glm::vec2 scaleLimits;

    virtual float GetRequiredAmmoToFire() override;
    virtual void FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection) override;

public:
    PressureWeapon::PressureWeapon(BasicPhysics* physics, glm::vec2 scaleLimits);
    void Render(const glm::mat4& projectionMatrix) override;
};

