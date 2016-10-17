#pragma once
#include <glm\vec3.hpp>
#include "AI\MilitaryStateMachine.h"
#include "Managers\NpcManager.h"
#include "Weapons\PlasmaWeapon.h"
#include "NPC.h"

class EnemyNPC : public NPC
{
    NpcManager* manager;
    MilitaryStateMachine state;

    PlasmaWeapon mainWeapon;

    bool actionPerformed;
    float timer;
    glm::vec3 target;
    unsigned int npcId;
public:
    EnemyNPC(NpcManager* manager, BasicPhysics* physics);

    bool IsTimerExpired() const;
    void TickTimer(float elapsedTime);

    bool IsNearTarget(float allowedOffset) const;
    bool IsLowPerceivedHealth() const;

    // Wraps calls into the NPC manager to check for nearby enemies.
    bool IsNearbyEnemy();
    bool AreManyNearbyEnemies();

    virtual std::string GetDescription() const override;
    virtual void Update(float gameTime, float elapsedTime) override;
    virtual void Render(FontManager* fontManager, ModelManager* modelManager, const glm::mat4& projectionMatrix) override;
};

