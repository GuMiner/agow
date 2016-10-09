#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include "Generators\NameGenerator.h"
#include "Generators\ColorGenerator.h"
#include "CivilianNPC.h"

CivilianNPC::CivilianNPC()
    : NPC(NameGenerator::GetRandomName(), "",
        (NPC::Shape)glm::linearRand(0, (int)NPC::Shape::COUNT - 1),
        ColorGenerator::GetCivilianColor(), 5), // TODO configurable
      state(State::Loitering)
{}

std::string CivilianNPC::GetDescription() const
{
    std::string stateString("acting unusually.");
    switch (state)
    {
    case State::Criminalizing:
        stateString = "breaking local laws.";
        break;
    case State::Fleeing:
        stateString = "running away";
        break;
    case State::Loitering:
        stateString = "hanging out.";
        break;
    case State::Traveling:
        stateString = "on the move.";
        break;
    case State::Vigilanteeing:
    default:
        stateString = "fighting crime.";
        break;
    }

    return std::string("Civilian is currently ") + stateString;
}

void CivilianNPC::Update(float gameTime, float elapsedTime)
{
    NPC::Update(gameTime, elapsedTime);

    // TODO perform action based on state.
}