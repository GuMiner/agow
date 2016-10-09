#pragma once
#include "NPC.h"

class CivilianNPC : public NPC
{
    // TODO move state machine to a class if it proves to be common.
    enum State
    {
        Loitering,
        Traveling,
        Fleeing,
        Criminalizing,
        Vigilanteeing
    };

    State state;

public:
    CivilianNPC();

    virtual std::string GetDescription() const override;
    virtual void Update(float gameTime, float elapsedTime) override;
};

