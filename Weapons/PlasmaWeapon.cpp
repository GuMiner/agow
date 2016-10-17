#include <algorithm>
#include "Math\PhysicsOps.h"
#include "Generators\RockGenerator.h"
#include "WeaponBase.h"
#include "PlasmaWeapon.h"

PlasmaProgram PlasmaWeapon::program;

PlasmaWeapon::PlasmaWeapon(BasicPhysics* physics)
    : WeaponBase(physics, "Plasma Fyre", 1000.0f, false, 0.3f, 0.0f),
      maxDistance(50.0f), maxProjectiles(100) // TODO configurable
{
}

float PlasmaWeapon::GetRequiredAmmoToFire()
{
    // TODO configurable.
    return 10.0f;
}

bool PlasmaWeapon::LoadGraphics(ShaderManager* shaderManager)
{
    // Sky program.
    if (!shaderManager->CreateShaderProgram("plasmaRender", &program.programId))
    {
        Logger::Log("Failure creating the plasma shader program!");
        return false;
    }

    program.projMatrixLocation = glGetUniformLocation(program.programId, "projMatrix");
    program.positionLocation = glGetUniformLocation(program.programId, "position");
    program.frameTimeLocation = glGetUniformLocation(program.programId, "frameTime");

    // We need the VAO to actually render without sending any vertex data to the shader, which handles the stars.
    glGenVertexArrays(1, &program.vao);
    glBindVertexArray(program.vao);

    return true;
}

void PlasmaWeapon::FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection)
{
    PlasmaProjectileData* projectile = new PlasmaProjectileData();
    projectile->body = physics->GetDynamicBody(BasicPhysics::CShape::WEAPON_PLASMA, PhysicsOps::Convert(fireOrigin), 1.0f);
    projectile->flightTime = 0.0f;

    glm::vec3 vel = 10.0f * fireDirection;
    projectile->body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
    projectile->body->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::PLASMA_BALL, this));
    // projectile->body->setCcdMotionThreshold(0.3f);
    // projectile->body->setCcdSweptSphereRadius(0.2f);

    // Save the rock to the known projectiles.
    while (projectiles.size() >= (unsigned int)maxProjectiles)
    {
        PlasmaProjectileData* projectileToRemove = (PlasmaProjectileData*)projectiles.front();

        delete projectileToRemove->body->getUserPointer();
        physics->DynamicsWorld->removeRigidBody(projectileToRemove->body);
        physics->DeleteBody(projectileToRemove->body, false);
        projectiles.pop_front();
    }

    physics->DynamicsWorld->addRigidBody(projectile->body);
    projectiles.push_back(projectile);
}

void PlasmaWeapon::Update(float elapsedTime)
{
    WeaponBase::Update(elapsedTime);
    for (auto iter = projectiles.cbegin(); iter != projectiles.cend(); iter++)
    {
        PlasmaProjectileData* projectile = (PlasmaProjectileData*)(*iter);
        projectile->flightTime += elapsedTime;
    }
}

void PlasmaWeapon::Render(const glm::mat4& projectionMatrix)
{
    // Render all the moving projectiles.
    for (auto iter = projectiles.cbegin(); iter != projectiles.cend(); iter++)
    {
        PlasmaProjectileData* projectile = (PlasmaProjectileData*)(*iter);
        glUseProgram(program.programId);
        glBindVertexArray(program.vao);

        glm::vec3 pos = BasicPhysics::GetBodyPosition(projectile->body);
        glUniform3f(program.positionLocation, pos.x, pos.y, pos.z);
        glUniform1f(program.frameTimeLocation, projectile->flightTime);
        glUniformMatrix4fv(program.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // 36 == cube.
        glDrawArrays(GL_TRIANGLES, 0, 1440);
    }
}

void PlasmaWeapon::Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData)
{
    // TODO have the plasma ball explode when it hits stuff.
}

void PlasmaWeapon::UnloadGraphics()
{
    glDeleteVertexArrays(1, &program.vao);
    glDeleteProgram(program.programId);
}
