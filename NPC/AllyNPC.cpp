#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include "Generators\NameGenerator.h"
#include "Generators\ColorGenerator.h"
#include "AllyNPC.h"

AllyNPC::AllyNPC()
    : NPC(NameGenerator::GetRandomName(), "",
        (NPC::Shape)glm::linearRand(0, (int)NPC::Shape::COUNT - 1),
        ColorGenerator::GetAllyColor(), 5) // TODO configurable
{
}

std::string AllyNPC::GetDescription() const
{
    return std::string("Ally is currently ") + state.GetStateDescription();
}

void AllyNPC::Update(float gameTime, float elapsedTime)
{
    NPC::Update(gameTime, elapsedTime);

    // TODO perform action based on state.
}