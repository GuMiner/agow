#pragma once
#include <string>
#include <deque>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\quaternion.hpp>
#include "BasicPhysics.h"

class WeaponBase
{
protected:
    BasicPhysics* physics;
    std::string name;
    
    float storedAmmo;
    float ammoLimit;

    bool continualFire;
    float fireRateInSeconds;
    float cooldownInSeconds; 
    
    float firingTime;
    float coolingTime;

    // Returns true if the weapon can fire or continue to fire.
    bool fireAttempt;
    bool CanFire();
    virtual float GetRequiredAmmoToFire() = 0;
    virtual void FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection) = 0;

    std::deque<void*> projectiles;

public:
    WeaponBase(BasicPhysics* physics, std::string name, float ammoLimit, bool continualFire, float fireRateInSeconds, float cooldownInSeconds);
    virtual void Update(float elapsedTime);
    
    // Reloads, returning the amount of ammo not loaded.
    float Reload(float newAmmo);
    void Fire(glm::vec3 fireOrigin, glm::vec3 fireDirection);
    
    virtual void Render(const glm::mat4& projectionMatrix) = 0;
};