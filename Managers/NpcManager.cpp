#include "Utils\Logger.h"
#include "Utils\TypedCallback.h"
#include "DialogPane.h"
#include "Map.h"
#include "NpcManager.h"

NpcManager::NpcManager()
{
    for (unsigned int i = 0; i < (unsigned int)NpcType::COUNT; i++)
    {
        npcs[(NpcType)i] = std::vector<NPC>();
    }

    // TODO make this not order-dependent.
    npcs[NpcType::KEY].push_back(NPC("James Blanton", "Giver of yer gear.", NPC::Shape::DIAMOND, 
        glm::vec4(0.0f, 1.0f, 0.10f, 0.80f), NPC::INVULNERABLE));
    npcs[NpcType::KEY].push_back(NPC("Aaron Krinst", "Giver of yer data.", NPC::Shape::DIAMOND,
        glm::vec4(0.0f, 0.20f, 1.0f, 0.70f), NPC::INVULNERABLE));
    npcs[NpcType::KEY].push_back(NPC("Barry Ingleson", "Nominal strategy director.", NPC::Shape::CUBOID,
        glm::vec4(1.0f, 0.10f, 0.0f, 0.90f), NPC::INVULNERABLE));
    npcs[NpcType::KEY].push_back(NPC("Oliver Yttrisk", "Battle assistant extraordinaire.", NPC::Shape::CUBOID,
        glm::vec4(1.0f, 0.50f, 0.0f, 0.50f), NPC::INVULNERABLE));
}

void NpcManager::LoadNpcPhysics(BasicPhysics physics, RegionManager* regionManager)
{
    // TODO configurable.
    glm::vec2 gearSciPos = Map::GetPoint(Map::POI::GEAR_SCIENTIST);
    glm::vec2 intelSciPos = Map::GetPoint(Map::POI::INTELLIGENCE_SCIENTIST);
    glm::vec2 generalMilPos = Map::GetPoint(Map::POI::GENERAL_MILITARY);
    glm::vec2 sergeantMilPos = Map::GetPoint(Map::POI::SERGEANT_MILITARY);
    
    npcs[NpcType::KEY][KeyNpcs::GEAR].LoadNpcPhysics(physics,
        glm::vec3(gearSciPos.x, gearSciPos.y, 2 + regionManager->GetPointHeight(physics.DynamicsWorld, gearSciPos)), 100);
    npcs[NpcType::KEY][KeyNpcs::INTEL].LoadNpcPhysics(physics,
        glm::vec3(intelSciPos.x, intelSciPos.y, 2 + regionManager->GetPointHeight(physics.DynamicsWorld, intelSciPos)), 90);
    npcs[NpcType::KEY][KeyNpcs::GENERAL].LoadNpcPhysics(physics,
        glm::vec3(generalMilPos.x, generalMilPos.y, 2 + regionManager->GetPointHeight(physics.DynamicsWorld, generalMilPos)), 80);
    npcs[NpcType::KEY][KeyNpcs::SERGEANT].LoadNpcPhysics(physics,
        glm::vec3(sergeantMilPos.x, sergeantMilPos.y, 2 + regionManager->GetPointHeight(physics.DynamicsWorld, sergeantMilPos)), 65);
}

void NpcManager::LoadGraphics(FontManager* fontManager)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            innerIter->LoadGraphics(fontManager);
        }
    }
}

void NpcManager::Converse(Events* events, DialogPane* dialogPane)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            if (innerIter->Converse(dialogPane))
            {
                // Successful conversation.
                Logger::Log("Talked to ", innerIter->GetName());
                return; 
            }
        }
    }

    // TODO configurable.
    dialogPane->QueueText(StyleText("There is no one to talk to.", StyleText::Effect::MINI));
    events->AddSingleEvent(TypedCallback<EventType>(EventType::ADD_DIALOG, dialogPane,
        new DialogData(StyleText("[Except for me].", StyleText::Effect::ITALICS)), true), 8.0f);
}

void NpcManager::Update(float gameTime, float frameTime)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            innerIter->Update(gameTime, frameTime);
        }
    }
}

void NpcManager::Render(FontManager* fontManager, ModelManager* modelManager, glm::mat4 projectionMatrix)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            innerIter->Render(fontManager, modelManager, projectionMatrix);
        }
    }
}

void NpcManager::UnloadNpcPhysics(BasicPhysics physics)
{
    for (auto iter = npcs.begin(); iter != npcs.end(); iter++)
    {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++)
        {
            innerIter->UnloadNpcPhysics(physics);
        }
    }
}
