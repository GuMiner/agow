#include <algorithm>
#include "Math\PhysicsOps.h"
#include "RockGenerator.h"
#include "WeaponBase.h"
#include "RockWeapon.h"

RockWeapon::RockWeapon(ModelManager* modelManager, BasicPhysics* physics, glm::vec2 speedLimits)
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

void RockWeapon::FireInternal(glm::vec3 fireOrigin, glm::quat fireDirection)
{
    // TODO add SFX and interact with everything else. Also manage projectiles list.
    PhysicalModel* model = new PhysicalModel();;
    BasicPhysics::CShape shape;

    RockGenerator rockGenerator;
    rockGenerator.GetRandomRockModel(&model->modelId, &shape);

    glm::vec3 vel = 40.0f * PhysicsOps::ForwardsVector(fireDirection);
    model->rigidBody = physics->GetDynamicBody(shape, btVector3(fireOrigin.x, fireOrigin.y, fireOrigin.z), 20.0f);
    model->rigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));

    // Save the rock to the known projectiles.
    while (projectiles.size() >= (unsigned int)maxProjectiles)
    {
        PhysicalModel* modelToRemove = (PhysicalModel*)projectiles.front();
        physics->DynamicsWorld->removeRigidBody(modelToRemove->rigidBody);
        physics->DeleteBody(modelToRemove->rigidBody, false);
        projectiles.pop_front();
    }

    physics->DynamicsWorld->addRigidBody(model->rigidBody);
    projectiles.push_back(model);
}

void RockWeapon::Render(const glm::mat4& projectionMatrix)
{
    // Render all the moving projectiles.
    for (auto iter = projectiles.cbegin(); iter != projectiles.cend(); iter++)
    {
        // For the rock weapon, the projectiles are simple models.
        PhysicalModel* model = (PhysicalModel*)(*iter);
        glm::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(model->rigidBody);
        modelManager->RenderModel(projectionMatrix, model->modelId, mvMatrix, false);
    }
}