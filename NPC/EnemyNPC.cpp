#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include "Generators\NameGenerator.h"
#include "Generators\ColorGenerator.h"
#include "EnemyNPC.h"

EnemyNPC::EnemyNPC()
    : NPC(NameGenerator::GetRandomName(), "",
        (NPC::Shape)glm::linearRand(0, (int)NPC::Shape::COUNT - 1),
        ColorGenerator::GetEnemyColor(), 5) // TODO configurable
{
}

std::string EnemyNPC::GetDescription() const
{
    return std::string("Enemy is currently ") + state.GetStateDescription();
}

void EnemyNPC::Update(float gameTime, float elapsedTime)
{
    NPC::Update(gameTime, elapsedTime);

    // TODO perform action based on state.
}