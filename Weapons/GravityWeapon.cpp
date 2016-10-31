#include <algorithm>
#include "WeaponBase.h"
#include "GravityWeapon.h"

GravityWeapon::GravityWeapon(Physics* physics)
    : WeaponBase(physics, "Asteroid Redirector", 1.0f, false, 60.0f, 0.0f) // TODO configurable
{
    storedAmmo = 1.0f;
}

float GravityWeapon::GetRequiredAmmoToFire()
{
    // Infinite ammo, but with an insanely long firing rate. Finding asteroids is slow work.
    return 0.0f;
}

void GravityWeapon::FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection)
{
    // TODO add SFX and interact with everything else. Also manage projectiles list.
}

void GravityWeapon::Render(const glm::mat4& projectionMatrix)
{

}