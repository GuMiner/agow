#include <algorithm>
#include "Utils\Logger.h"
#include "WeaponBase.h"

WeaponBase::WeaponBase(Physics* physics, std::string name, float ammoLimit, bool continualFire, float fireRateInSeconds, float cooldownInSeconds)
    : physics(physics), name(name), storedAmmo(0), coolingTime(0.0f), firingTime(0.0f), fireAttempt(false),
      ammoLimit(ammoLimit), continualFire(continualFire), fireRateInSeconds(fireRateInSeconds), cooldownInSeconds(cooldownInSeconds)
{
}

void WeaponBase::Update(float elapsedTime)
{
    firingTime = std::max(0.0f, firingTime - elapsedTime);
    if (continualFire && !fireAttempt)
    {
        // Set the firing time to zero if this is a continual fire weapon
        firingTime = 0.0f;
    }

    if (firingTime <= 0.0f)
    {
        coolingTime = std::max(0.0f, coolingTime - elapsedTime);
    }

    fireAttempt = false;
}

float WeaponBase::Reload(float newAmmo)
{
    float extraAmmo = std::max(0.0f, newAmmo - (ammoLimit - storedAmmo));
    storedAmmo = std::min(ammoLimit, storedAmmo + newAmmo);

    return extraAmmo;
}

bool WeaponBase::CanFire()
{
    // We cannot fire if we are low on ammo.
    if (storedAmmo < GetRequiredAmmoToFire())
    {
        Logger::Log("Out of ammo for weapon ", name);
        return false;
    }

    // ... or the weapon is cooling down.
    if (coolingTime > 0.0f)
    {
        return false;
    }

    // ... or we fired too recently
    if (firingTime > 0.0f)
    {
        return false;
    }

    return true;
}

void WeaponBase::Fire(glm::vec3 fireOrigin, glm::vec3 fireDirection)
{
    fireAttempt = true;
    if (CanFire())
    {
        storedAmmo -= GetRequiredAmmoToFire();
        firingTime = fireRateInSeconds;
        coolingTime = cooldownInSeconds;
        FireInternal(fireOrigin, fireDirection);
    }
    else if (continualFire && firingTime > 0.0f)
    {
        // Continue fire weapons we can fire until firing time == 0, at which point we have to wait for the cooldown.
        FireInternal(fireOrigin, fireDirection);
    }
}
