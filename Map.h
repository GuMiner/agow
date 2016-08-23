#pragma once
#include "Math\Vec.h"

class Map
{
public:
	enum POI
	{
		PLAYER,
		GEAR_SCIENTIST,
		INTELLIGENCE_SCIENTIST,
		GENERAL_MILITARY,
		SERGEANT_MILITARY
	};

private:
	

public:
	Map();

	// For player POI, this returns their initial point at game start.
	// For NPC POI, this returns the NPC initial point.
	// Positions are in real coordinates.
	static vec::vec2 GetPoint(POI point);

	virtual ~Map();
};

