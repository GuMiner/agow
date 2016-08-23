#include "Map.h"

Map::Map()
{
}

vec::vec2 Map::GetPoint(POI point)
{
	switch (point)
	{
	case POI::PLAYER:
		return vec::vec2(12000, 44000);
	case POI::GEAR_SCIENTIST:
		return vec::vec2(12020, 44000);
	case POI::INTELLIGENCE_SCIENTIST:
		return vec::vec2(12000, 44040);
	case POI::GENERAL_MILITARY:
		return vec::vec2(11980, 44000);
	case POI::SERGEANT_MILITARY:
		return vec::vec2(12000, 43980);
	default:
		return vec::vec2(12000, 44000);
	}
}

Map::~Map()
{
}
