#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "TerrainEffects\CityEffect.h"
#include "TerrainEffects\GrassEffect.h"
#include "TerrainEffects\RoadEffect.h"
#include "TerrainEffects\RockEffect.h"
#include "TerrainEffects\SignEffect.h"
#include "TerrainEffects\TreeEffect.h"
#include "logging\Logger.h"
#include "Utils\ImageUtils.h"


TerrainEffectManager::TerrainEffectManager(ShaderFactory* shaderManager, ModelManager* modelManager, Physics* Physics)
    : shaderManager(shaderManager), modelManager(modelManager), physics(Physics)
{
    effects.push_back((TerrainEffect*)new GrassEffect());
    effects.push_back((TerrainEffect*)new RockEffect(modelManager, physics));
    effects.push_back((TerrainEffect*)new RoadEffect());
    effects.push_back((TerrainEffect*)new SignEffect(modelManager, physics));
    // 
    // // TODO configurable
    effects.push_back((TerrainEffect*)new TreeEffect("cache/trees"));
    effects.push_back((TerrainEffect*)new CityEffect(modelManager, physics, "cache/buildings"));
}

bool TerrainEffectManager::LoadBasics()
{
    for (auto iter = effects.begin(); iter != effects.end(); iter++)
    {
        if (!(*iter)->LoadBasics(shaderManager))
        {
            return false;
        }
    }

    return true;
}

bool TerrainEffectManager::LoadSubTileEffects(glm::ivec2 start, SubTile* tile)
{
    if (subtileEffectData.find(start) != subtileEffectData.end())
    {
        // Already in cache.
        return true;
    }

    std::vector<TerrainEffectData*> tileEffects;
    for (auto iter = effects.begin(); iter != effects.end(); iter++)
    {
        void* effectData;
        if ((*iter)->LoadEffect(start, &effectData, tile))
        {
            tileEffects.push_back(new TerrainEffectData(*iter, effectData));
        }
    }

    subtileEffectData[start] = tileEffects;
    return true;
}

void TerrainEffectManager::Simulate(const glm::ivec2 start, float elapsedSeconds)
{
    if (subtileEffectData.find(start) == subtileEffectData.end())
    {
        Logger::LogWarn("Attempted to simulate terrain effects not loaded with [", start.x, ", ", start.y, "].");
        return;
    }

    for (auto iter = subtileEffectData[start].begin(); iter != subtileEffectData[start].end(); iter++)
    {
        (*iter)->effect->Simulate(start, (*iter)->effectData, elapsedSeconds);
    }
}

void TerrainEffectManager::RenderSubTileEffects(const glm::ivec2 start, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    if (subtileEffectData.find(start) == subtileEffectData.end())
    {
        Logger::LogWarn("Attempted to render terrain effects not loaded with [", start.x, ", ", start.y, "].");
        return;
    }

    for (auto iter = subtileEffectData[start].begin(); iter != subtileEffectData[start].end(); iter++)
    {
        (*iter)->effect->Render((*iter)->effectData, perspectiveMatrix, viewMatrix, modelMatrix);
    }
}

void TerrainEffectManager::LogEffectInformation()
{
    for (auto iter = effects.begin(); iter != effects.end(); iter++)
    {
        (*iter)->LogStats();
    }
}

void TerrainEffectManager::CleanupSubTileEffects(glm::ivec2 start, bool log)
{
    for (auto iter = subtileEffectData[start].begin(); iter != subtileEffectData[start].end(); iter++)
    {
        (*iter)->effect->UnloadEffect((*iter)->effectData);
        delete (*iter);
    }

    if (log)
    {
        Logger::Log("Unloaded effects ", start.x, ", ", start.y, ".");
    }
}

void TerrainEffectManager::UnloadSubTileEffects(glm::ivec2 start)
{
    CleanupSubTileEffects(start, true);
    subtileEffectData.erase(start);
}

TerrainEffectManager::~TerrainEffectManager()
{
    // Cleanup any allocated effects 
    for (auto iter = subtileEffectData.begin(); iter != subtileEffectData.end(); iter++)
    {
        CleanupSubTileEffects(iter->first, false);
    }

    // Cleanup the classes running the effects themselves.
    for (auto iter = effects.begin(); iter != effects.end(); iter++)
    {
        delete *iter;
    }
}
