#include <algorithm>
#include "WeaponBase.h"
#include "SunbeamWeapon.h"

SunbeamWeapon::SunbeamWeapon(BasicPhysics* physics)
    : WeaponBase(physics, "Solar Concentrator", 1.0f, true, 3.0f, 5.0f) // TODO configurable
{
    storedAmmo = 1.0f;
}

float SunbeamWeapon::GetRequiredAmmoToFire()
{
    // Infinite ammo.
    return 0.0f;
}

void SunbeamWeapon::FireInternal(glm::vec3 fireOrigin, glm::quat fireDirection)
{
    // TODO add SFX and interact with everything else. Also manage projectiles list.
}

void SunbeamWeapon::Render(const glm::mat4& projectionMatrix)
{

}