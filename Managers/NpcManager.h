#pragma once
#include <map>
#include <string>
#include <vector>
#include <glm\vec3.hpp>
#include "Managers\FontManager.h"
#include "Managers\ModelManager.h"
#include "Managers\RegionManager.h"
#include "Physics.h"
#include "Events.h"
#include "Player.h"
#include "NPC\Npc.h"

enum NpcType : unsigned int;

struct NpcTarget
{
    bool targetSet;
    NpcType targetType;
    unsigned int targetNpcId;

    NpcTarget()
        : targetSet(false)
    {
    }
};

// Defines loading and displaying a single unit of terrain.
class NpcManager
{
public:
    enum NpcType : unsigned int
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
    Player* player;

    std::map<NpcType, std::vector<NPC*>> npcs;
    std::map<NpcType, NpcTarget> targets;

public:
    static const int PLAYER_ID = -1;

    NpcManager(Player* player, Physics* physics);
    void LoadNpcPhysics(Physics* physics, RegionManager* regionManager);
    void LoadGraphics(FontManager* fontManager);
    
    void Converse(Events* events, DialogPane* dialogPane);
    void Update(float gameTime, float frameTime);
    void Render(FontManager* fontManager, ModelManager* modelManager, glm::mat4 projectionMatrix);
    
    void UnloadNpcPhysics(Physics* physics);
    
    // NpcType == type of item performing the query.
    bool IsTargettedEnemy(NpcType npcType, unsigned int* npcId);
    bool IsTargettedAlly(NpcType npcType, unsigned int* npcId);

    // NpcType == type of item being queried.
    bool IsNearbyEnemy(const glm::vec3& pos, float radius, NpcType npcType, unsigned int* enemyId);
    bool AreManyNearbyEnemies(const glm::vec3& pos, float radius, int manyThreshold, NpcType npcType, unsigned int* singularEnemyId);
    bool IsEnemyAlive(NpcType npcType, unsigned int* npcId);
    glm::vec3 GetNpcPosition(NpcType npcType, unsigned int npcId);
};

