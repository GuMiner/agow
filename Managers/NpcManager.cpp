#include "NPC\AllyNPC.h"
#include "NPC\EnemyNPC.h"
#include "Utils\Logger.h"
#include "Utils\TypedCallback.h"
#include "DialogPane.h"
#include "Map.h"
#include "NpcManager.h"

NpcManager::NpcManager(Player* player, Physics* physics)
    : player(player)
{
    for (unsigned int i = 0; i < (unsigned int)NpcType::COUNT; i++)
    {
        npcs[(NpcType)i] = std::vector<NPC*>();
        targets[(NpcType)i] = NpcTarget();
    }

    // TODO make this not order-dependent.
    npcs[NpcType::KEY].push_back(new NPC("James Blanton", "Giver of yer gear.", NPC::Shape::DIAMOND,
        glm::vec4(0.0f, 1.0f, 0.10f, 0.80f), NPC::INVULNERABLE));
    npcs[NpcType::KEY].push_back(new NPC("Aaron Krinst", "Giver of yer data.", NPC::Shape::DIAMOND,
        glm::vec4(0.0f, 0.20f, 1.0f, 0.70f), NPC::INVULNERABLE));
    npcs[NpcType::KEY].push_back(new NPC("Barry Ingleson", "Nominal strategy director.", NPC::Shape::CUBOID,
        glm::vec4(1.0f, 0.10f, 0.0f, 0.90f), NPC::INVULNERABLE));
    npcs[NpcType::KEY].push_back(new NPC("Oliver Yttrisk", "Battle assistant extraordinaire.", NPC::Shape::CUBOID,
        glm::vec4(1.0f, 0.50f, 0.0f, 0.50f), NPC::INVULNERABLE));

    npcs[NpcType::ENEMY].push_back(new EnemyNPC(this, physics));
    npcs[NpcType::ALLY].push_back(new AllyNPC());
}

void NpcManager::LoadNpcPhysics(Physics* physics, RegionManager* regionManager)
{
    // TODO configurable.
    glm::vec2 gearSciPos = Map::GetPoint(Map::POI::GEAR_SCIENTIST);
    glm::vec2 intelSciPos = Map::GetPoint(Map::POI::INTELLIGENCE_SCIENTIST);
    glm::vec2 generalMilPos = Map::GetPoint(Map::POI::GENERAL_MILITARY);
    glm::vec2 sergeantMilPos = Map::GetPoint(Map::POI::SERGEANT_MILITARY);
    
    npcs[NpcType::KEY][KeyNpcs::GEAR]->LoadNpcPhysics(physics,
        glm::vec3(gearSciPos.x, gearSciPos.y, 2 + regionManager->GetPointHeight(physics, gearSciPos)), 100);
    npcs[NpcType::KEY][KeyNpcs::INTEL]->LoadNpcPhysics(physics,
        glm::vec3(intelSciPos.x, intelSciPos.y, 2 + regionManager->GetPointHeight(physics, intelSciPos)), 90);
    npcs[NpcType::KEY][KeyNpcs::GENERAL]->LoadNpcPhysics(physics,
        glm::vec3(generalMilPos.x, generalMilPos.y, 2 + regionManager->GetPointHeight(physics, generalMilPos)), 80);
    npcs[NpcType::KEY][KeyNpcs::SERGEANT]->LoadNpcPhysics(physics,
        glm::vec3(sergeantMilPos.x, sergeantMilPos.y, 2 + regionManager->GetPointHeight(physics, sergeantMilPos)), 65);

    // TODO test code remove
    glm::vec2 enemyPos = generalMilPos + glm::vec2(3.0f, 3.0f);
    npcs[NpcType::ENEMY][0]->LoadNpcPhysics(physics, glm::vec3(enemyPos.x, enemyPos.y, 2 + regionManager->GetPointHeight(physics, enemyPos)), 200.0f);

    glm::vec2 allyPos = generalMilPos - glm::vec2(8.0f, 8.0f);
    npcs[NpcType::ALLY][0]->LoadNpcPhysics(physics, glm::vec3(allyPos.x, allyPos.y, 2 + regionManager->GetPointHeight(physics, allyPos)), 70.0f);
}

void NpcManager::LoadGraphics(FontManager* fontManager)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            (*innerIter)->LoadGraphics(fontManager);
        }
    }
}

void NpcManager::Converse(Events* events, DialogPane* dialogPane)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            if ((*innerIter)->Converse(dialogPane))
            {
                // Successful conversation.
                Logger::Log("Talked to ", (*innerIter)->GetName());
                return; 
            }
        }
    }

    // TODO configurable.
    dialogPane->QueueText(StyleText("[HUD]", "There is no one to talk to.", StyleText::Effect::MINI));
    events->AddSingleEvent(TypedCallback<EventType>(EventType::ADD_DIALOG, dialogPane,
        new DialogData(StyleText("[HUD]", "[Except for me].", StyleText::Effect::ITALICS)), true), 8.0f);
}

void NpcManager::Update(float gameTime, float frameTime)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            (*innerIter)->Update(gameTime, frameTime);
        }
    }
}

void NpcManager::Render(FontManager* fontManager, ModelManager* modelManager, glm::mat4 projectionMatrix)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            (*innerIter)->Render(fontManager, modelManager, projectionMatrix);
        }
    }
}

void NpcManager::UnloadNpcPhysics(Physics* physics)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            (*innerIter)->UnloadNpcPhysics(physics);
        }
    }
}

bool NpcManager::IsTargettedEnemy(NpcType npcType, unsigned int* npcId)
{
    if (targets[npcType].targetSet)
    {
        if (targets[npcType].targetType != npcType)
        {
            *npcId = targets[npcType].targetNpcId;
            return true;
        }
    }

    return false;
}

bool NpcManager::IsTargettedAlly(NpcType npcType, unsigned int* npcId)
{
    if (targets[npcType].targetSet)
    {
        if (targets[npcType].targetType == npcType)
        {
            *npcId = targets[npcType].targetNpcId;
            return true;
        }
    }

    return false;
}

bool NpcManager::IsNearbyEnemy(const glm::vec3& pos, float radius, NpcType npcType, unsigned int* enemyId)
{
    return AreManyNearbyEnemies(pos, radius, 1, npcType, enemyId);
}

bool NpcManager::AreManyNearbyEnemies(const glm::vec3& pos, float radius, int manyThreshold, NpcType npcType, unsigned int* singularEnemyId)
{
    if (npcs[npcType].size() >= (unsigned int)manyThreshold)
    {
        int count = 0;
        for (unsigned int i = 0; i < npcs[npcType].size(); i++)
        {
            if (glm::distance(npcs[npcType][i]->GetPosition(), pos) < radius)
            {
                ++count;
                *singularEnemyId = i;

                if (count >= manyThreshold)
                {
                    return true;
                }
            }
        }
    }

    return false;
}


bool NpcManager::IsEnemyAlive(NpcType npcType, unsigned int* npcId)
{
    if (*npcId == PLAYER_ID)
    {
        // If the player is dead, we have our own handling logic.
        return false;
    }

    if (*npcId >= npcs[npcType].size())
    {
        Logger::LogWarn("IsEnemyAlive of NpcManager attempted to get an NPC off the end of the list: [", npcType, ",  ", *npcId, "].");
        return true;
    }

    return npcs[npcType][*npcId]->IsAlive();
}

glm::vec3 NpcManager::GetNpcPosition(NpcType npcType, unsigned int npcId)
{
    if (npcId == PLAYER_ID)
    {
        return player->GetPosition();
    }

    if (npcId >= npcs[npcType].size())
    {
        Logger::LogWarn("GetNpcPosition of NpcManager attempted to get an NPC off the end of the list: [", npcType, ",  ", npcId, "].");
        return player->GetPosition();
    }

    return npcs[npcType][npcId]->GetPosition();
}