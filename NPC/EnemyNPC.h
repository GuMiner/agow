#pragma once
#include "AI\MilitaryStateMachine.h"
#include "NPC.h"

class EnemyNPC : public NPC
{
    MilitaryStateMachine state;

public:
    EnemyNPC();

    virtual std::string GetDescription() const override;
    virtual void Update(float gameTime, float elapsedTime) override;
};

