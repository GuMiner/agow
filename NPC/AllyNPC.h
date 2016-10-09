#pragma once
#include "AI\MilitaryStateMachine.h"
#include "NPC.h"

class AllyNPC : public NPC
{
    MilitaryStateMachine state;

public:
    AllyNPC();

    virtual std::string GetDescription() const override;
    virtual void Update(float gameTime, float elapsedTime) override;
};

