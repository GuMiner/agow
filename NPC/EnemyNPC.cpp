#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include "Generators\NameGenerator.h"
#include "Generators\ColorGenerator.h"
#include "Math\PhysicsOps.h"
#include "BasicPhysics.h"
#include "EnemyNPC.h"

EnemyNPC::EnemyNPC(NpcManager* manager, BasicPhysics* physics)
    : NPC(NameGenerator::GetRandomName(), "",
        (NPC::Shape)glm::linearRand(0, (int)NPC::Shape::COUNT - 1),
        ColorGenerator::GetEnemyColor(), 5), // TODO configurable
      mainWeapon(physics), manager(manager), state(GetName()), timer(0.0f), target(0.0f)
{
    std::function<bool()> hasThrownExplosive = std::bind(&EnemyNPC::IsTimerExpired, this);
    std::function<bool()> hasCalledForBackup = std::bind(&EnemyNPC::IsTimerExpired, this);
    std::function<bool()> hasReachedTarget = std::bind(&EnemyNPC::IsNearTarget, this, 1.0f);
    std::function<bool()> hasReachedAlly = std::bind(&EnemyNPC::IsNearTarget, this, 2.0f);

    std::function<bool()> isNearbyEnemy = std::bind(&EnemyNPC::IsNearbyEnemy, this);
    std::function<bool()> isTargettedEnemy = std::bind(&NpcManager::IsTargettedEnemy, manager, NpcManager::NpcType::ENEMY, &npcId);
    std::function<bool()> isTargettedAlly = std::bind(&NpcManager::IsTargettedAlly, manager, NpcManager::NpcType::ENEMY, &npcId);
    std::function<bool()> isEnemyStillAlive = std::bind(&NpcManager::IsEnemyAlive, manager, NpcManager::NpcType::ALLY, &npcId);
    std::function<bool()> areManyNearbyEnemies = std::bind(&EnemyNPC::AreManyNearbyEnemies, this);
    std::function<bool()> isLowPerceivedHealth = std::bind(&EnemyNPC::IsLowPerceivedHealth, this);

    state.SetCallbackFunctions(hasReachedTarget, hasReachedAlly, hasThrownExplosive, hasCalledForBackup,
        isNearbyEnemy, isTargettedEnemy, isTargettedAlly, isEnemyStillAlive, areManyNearbyEnemies, isLowPerceivedHealth);

    // Enemies have nearly infinite ammo.
    mainWeapon.Reload(2000000);
}

bool EnemyNPC::IsTimerExpired() const
{
    return timer <= 0;
}

void EnemyNPC::TickTimer(float elapsedTime)
{
    if (timer > 0.0f)
    {
        timer -= elapsedTime;
        if (timer < 0.0f)
        {
            timer = 0.0f;
        }
    }
}

bool EnemyNPC::IsLowPerceivedHealth() const
{
    // TODO configurable.
    return startingHealth / health >= 5; // less than 20%, rounding up.
}

bool EnemyNPC::IsNearTarget(float allowedOffset) const
{
    return glm::distance(GetPosition(), target) < allowedOffset;
}

bool EnemyNPC::IsNearbyEnemy()
{
    // TODO configurable.
    return manager->IsNearbyEnemy(GetPosition(), 20.0f, NpcManager::NpcType::ALLY, &npcId);
}

bool EnemyNPC::AreManyNearbyEnemies()
{
    // TODO configurable.
    return manager->AreManyNearbyEnemies(GetPosition(), 20.0f, 4, NpcManager::NpcType::ALLY, &npcId);
}

std::string EnemyNPC::GetDescription() const
{
    return name + std::string(" is ") + state.GetStateDescription();
}

void EnemyNPC::Update(float gameTime, float elapsedTime)
{
    NPC::Update(gameTime, elapsedTime);
    state.Update(elapsedTime);

    TickTimer(elapsedTime);

    // TODO need to determine how to deal with the current state, and also state transitions.
    if (state.HasChangedState())
    {
        // Handle the new state.
        switch (state.GetState())
        {
        case MilitaryStateMachine::State::CallForBackup:
            timer = 5.0f; // TODO write animation for this scenario.
            actionPerformed = false;
            break;
        case MilitaryStateMachine::State::FireAtNearest:
        case MilitaryStateMachine::State::FireAtTargetted:
        case MilitaryStateMachine::State::Flee:
        case MilitaryStateMachine::State::Follow:
        case MilitaryStateMachine::State::Survey:
            // No setup required.
            break;
        case MilitaryStateMachine::State::TakeCover:
            // Determine where to take cover.
            // TODO this is kinda useless as-is.
            target = GetPosition() - glm::vec3(5.0f, 5.0f, 0.0f);
            break;
        case MilitaryStateMachine::State::ThrowExplosive:
            timer = 3.0f; // TODO write animation for when the enemy is throwing the explosive.
            actionPerformed = false;
            break;
        default:
            break;
        }
    }
    else
    {
        glm::vec3 projectileOrigin;
        glm::vec3 direction;

        // Handle the current state
        switch (state.GetState())
        {
        case MilitaryStateMachine::State::CallForBackup:
            if (timer < 2.0f && !actionPerformed)
            {
                // Actually call for backup
                // TODO indicate to the NPC manager to spawn a vehicle with enemies.
                actionPerformed = true;
            }

            break;
        case MilitaryStateMachine::State::FireAtNearest:
        case MilitaryStateMachine::State::FireAtTargetted:
            // Attempt to fire at the target, which will be nearest.
            target = manager->GetNpcPosition(NpcManager::NpcType::ALLY, npcId);
            
            // TODO configurable.
            projectileOrigin = GetPosition();
            direction = glm::normalize(target - projectileOrigin);
            projectileOrigin += (direction * 2.0f);

            mainWeapon.Fire(projectileOrigin, direction);
            break;
        case MilitaryStateMachine::State::Flee:
            target = manager->GetNpcPosition(NpcManager::NpcType::ALLY, npcId);
            
            // Thrust away from the enemy. TODO configurable.
            model.body->applyCentralForce(PhysicsOps::Convert(glm::normalize(GetPosition() - target) * 3000.0f));
            break;
        case MilitaryStateMachine::State::Follow:
            target = manager->GetNpcPosition(NpcManager::NpcType::ENEMY, npcId);
            
            // Thrust towards the ally we're following. TODO configurable 
            model.body->applyCentralForce(PhysicsOps::Convert(glm::normalize(target - GetPosition()) * 2000.0f));
            break;
        case MilitaryStateMachine::State::Survey:
            // TODO idle animations
            break;
        case MilitaryStateMachine::State::TakeCover:
            // Move towards the target. TODO configurable.
            model.body->applyCentralForce(PhysicsOps::Convert(glm::normalize(target - GetPosition()) * 1000.0f));
            break;
        case MilitaryStateMachine::State::ThrowExplosive:
            if (timer < 1.5f && !actionPerformed)
            {
                // Actually throw the explosive.
                // TODO create a physics object for this. 
                actionPerformed = true;
            }

            break;
        default:
            break;
        }
    }

    mainWeapon.Update(elapsedTime);
}

void EnemyNPC::Render(FontManager* fontManager, ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
    NPC::Render(fontManager, modelManager, projectionMatrix);
    mainWeapon.Render(projectionMatrix);
}