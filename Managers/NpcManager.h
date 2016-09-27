#pragma once
#include <map>
#include <string>
#include <vector>
#include "Managers\FontManager.h"
#include "Managers\ModelManager.h"
#include "Managers\RegionManager.h"
#include "Math\Vec.h"
#include "BasicPhysics.h"
#include "Events.h"
#include "Npc.h"

// Defines loading and displaying a single unit of terrain.
class NpcManager
{
public:
    enum NpcType
    {
        KEY,
        ENEMY,
        ALLY,
        CIVILIAN,
        COUNT
    };
    
    enum KeyNpcs
    {
        GEAR = 0,
        INTEL = 1,
        GENERAL = 2,
        SERGEANT = 3
    };

private:
    std::map<NpcType, std::vector<NPC>> npcs;

public:
    NpcManager();
    void LoadNpcPhysics(BasicPhysics physics, RegionManager* regionManager);
    void LoadGraphics(FontManager* fontManager);
    
    void Converse(Events* events, DialogPane* dialogPane);
    void Update(float gameTime, float frameTime);
    void Render(FontManager* fontManager, ModelManager* modelManager, vec::mat4 projectionMatrix);
    
    void UnloadNpcPhysics(BasicPhysics physics);
};

