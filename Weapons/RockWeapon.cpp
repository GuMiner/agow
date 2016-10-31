#include <algorithm>
#include "Math\PhysicsOps.h"
#include "Generators\PhysicsGenerator.h"
#include "Generators\RockGenerator.h"
#include "WeaponBase.h"
#include "RockWeapon.h"

RockWeapon::RockWeapon(ModelManager* modelManager, Physics* physics, glm::vec2 speedLimits)
    : WeaponBase(physics, "Rock Ejector", 1000.0f, false, 0.2f, 0.0f),
      speedLimits(speedLimits), sizeSetting(SizeSetting::BOULDER), modelManager(modelManager) // TODO configurable
{
    storedAmmo = 20000.0f;
    speed = speedLimits.x;
    
    // TODO configurable.
    maxProjectiles = 50;
}

float RockWeapon::GetRequiredAmmoToFire()
{
    switch (sizeSetting)
    {
    case BULLET:
        return 5.0f;
    case BOULDER:
        return 20.0f;
    case BUILDING:
    default:
        return 100.0f;
    }
}

void RockWeapon::FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection)
{
    // TODO add SFX and interact with everything else. Also manage projectiles list.
    Model* model = new Model();
    PhysicsGenerator::CShape shape;

    RockGenerator rockGenerator;
    rockGenerator.GetRandomRockModel(&model->modelId, &shape);

    glm::vec3 vel = 40.0f * fireDirection;
    model->body = PhysicsGenerator::GetDynamicBody(shape, btVector3(fireOrigin.x, fireOrigin.y, fireOrigin.z), 20.0f);
    model->body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
    model->body->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::ROCK, this));

    // Save the rock to the known projectiles.
    while (projectiles.size() >= (unsigned int)maxProjectiles)
    {
        Model* modelToRemove = (Model*)projectiles.front();

        physics->RemoveBody(modelToRemove->body);
        physics->DeleteBody(modelToRemove->body, false);
        projectiles.pop_front();
    }

    physics->AddBody(model->body);
    projectiles.push_back(model);
}

void RockWeapon::Render(const glm::mat4& projectionMatrix)
{
    // Render all the moving projectiles.
    for (auto iter = projectiles.cbegin(); iter != projectiles.cend(); iter++)
    {
        // For the rock weapon, the projectiles are simple models.
        modelManager->RenderModel(projectionMatrix, (Model*)(*iter));
    }
}

void RockWeapon::Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData)
{
    // TODO does the rock explode when it hits stuff? Possibilities...
}
