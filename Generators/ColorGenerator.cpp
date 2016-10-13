#include <glm\gtc\random.hpp>
#include "ColorGenerator.h"

// TODO TODO configurable for all of these methods.
glm::vec4 ColorGenerator::GetRockColor()
{
    // TODO implement
    return glm::vec4(1.0f);
}

glm::vec4 ColorGenerator::GetGrassColor()
{
    // TODO implement
    return glm::vec4(1.0f);
}

glm::vec4 ColorGenerator::GetTravellerColor()
{
    // TODO implement
    return glm::vec4(1.0f);
}

glm::vec3 ColorGenerator::GetTreeBranchColor()
{
    return glm::vec3(0.57f, 0.20f + glm::linearRand(0.0f, 0.30f), 0.10f + glm::linearRand(0.0f, 0.40f));
}

glm::vec3 ColorGenerator::GetTreeLeafColor()
{
    return glm::vec3(0.1f, 0.70f + glm::linearRand(0.0f, 0.30f), glm::linearRand(0.0f, 0.20f));
}

glm::vec4 ColorGenerator::GetAllyColor()
{
    // Allies are varying shades of green.
    float redAndBlue = glm::linearRand(0.0f, 0.40f);
    return glm::vec4(redAndBlue, 0.70f + glm::linearRand(0.0f, 0.30f), redAndBlue, 1.0f);
}

glm::vec4 ColorGenerator::GetCivilianColor()
{
    // Civilians are shades of blue.
    float redAndGreen = glm::linearRand(0.0f, 0.40f);
    return glm::vec4(redAndGreen, redAndGreen, 0.70f + glm::linearRand(0.0f, 0.30f), 1.0f);
}

glm::vec4 ColorGenerator::GetEnemyColor()
{
    // Enemies are shades of red.
    float greenAndBlue = glm::linearRand(0.0f, 0.40f);
    return glm::vec4(0.70f + glm::linearRand(0.0f, 0.30f), greenAndBlue, greenAndBlue, 1.0f);
}