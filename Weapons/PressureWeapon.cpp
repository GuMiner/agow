#include <algorithm>
#include "WeaponBase.h"
#include "PressureWeapon.h"

PressureWeapon::PressureWeapon(Physics* physics, glm::vec2 scaleLimits)
    : WeaponBase(physics, "Air Force", 100.0f, false, 1.0f, 0.0f), scaleLimits(scaleLimits) // TODO configurable
{
    storedAmmo = 0.0f;
    scale = scaleLimits.x;
}

float PressureWeapon::GetRequiredAmmoToFire()
{
    // Ammo usage is correlated to scale cubed.
    
    // TODO configurable.
    return std::pow(scale, 3.0f);
}

void PressureWeapon::FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection)
{
    // TODO add SFX and interact with everything else. Also manage projectiles list.
}

void PressureWeapon::Render(const glm::mat4& projectionMatrix)
{

}