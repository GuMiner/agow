#pragma once
#include <glm\vec4.hpp>

// Generates random colors that fit the overall color theme.
class ColorGenerator
{
public:
    // Effects
    static glm::vec4 GetRockColor();
    static glm::vec4 GetGrassColor();
    static glm::vec4 GetTravellerColor();

    // NPCs
    static glm::vec4 GetAllyColor();
    static glm::vec4 GetCivilianColor();
    static glm::vec4 GetEnemyColor();
};

