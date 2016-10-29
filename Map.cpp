#include "Map.h"

Map::Map()
{
}

glm::vec2 Map::GetPoint(POI point)
{
    // TODO configurable.
    switch (point)
    {
    case POI::PLAYER:
        return glm::vec2(12200, 44000);
    case POI::GEAR_SCIENTIST:
        return glm::vec2(12020, 44000);
    case POI::INTELLIGENCE_SCIENTIST:
        return glm::vec2(12000, 44040);
    case POI::GENERAL_MILITARY:
        return glm::vec2(11980, 44000);
    case POI::SERGEANT_MILITARY:
        return glm::vec2(12000, 43980);
    default:
        return glm::vec2(12000, 44000);
    }
}

Map::~Map()
{
}
