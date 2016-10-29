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

glm::vec3 ColorGenerator::GetTravellerColor()
{
    const glm::vec3 low(22, 85, 148);
    const glm::vec3 high(182, 213, 243);
    float shadingFactor = glm::linearRand(0.0f, 1.0f);
    return (low + (high - low) * shadingFactor) / 255.0f;
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

glm::vec4 ColorGenerator::GetBuildingColor()
{
    float alpha = 0.70f + glm::linearRand(0.0f, 0.20f);

    // Buildings are silver and gold. 
    bool isSilver = glm::linearRand(0.0f, 1.0f) > 0.40f;
    if (isSilver)
    {
        float shadingColor = glm::linearRand(0.55f, 0.85f);
        return glm::vec4(shadingColor, shadingColor, shadingColor, alpha);
    }
    else
    {
        float scale = glm::linearRand(0.0f, 1.0f);
        return glm::vec4(0.52f + scale * 0.292f, 0.607f + scale * 0.13f, 0.263f + scale * 0.349f, alpha);
    }
}