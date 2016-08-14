#pragma once
#include <string>
#include <vector>
#include "Managers\ConfigManager.h"

class PhysicsConfig : public ConfigManager
{
    virtual bool LoadConfigValues(std::vector<std::string>& lines);
    virtual void WriteConfigValues();
public:
    static int PhysicsThreadDelay;
    static float ViewForwardsSpeed;
    static float ViewSidewaysSpeed;

    static float ViewRotateUpFactor;
    static float ViewRotateAroundFactor;

    static int TerrainSize;

    PhysicsConfig(const char* configName);
};

