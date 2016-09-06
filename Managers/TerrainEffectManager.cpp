#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Math\MathOps.h"
#include "TerrainEffects\GrassEffect.h"
#include "TerrainEffects\RoadEffect.h"
#include "TerrainEffects\RockEffect.h"
#include "TerrainEffects\TreeEffect.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"


TerrainEffectManager::TerrainEffectManager(ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* basicPhysics, int subTileSize)
    : shaderManager(shaderManager), modelManager(modelManager), physics(basicPhysics), subTileSize(subTileSize)
{
    effects.push_back((TerrainEffect*)new GrassEffect(subTileSize));
    effects.push_back((TerrainEffect*)new RockEffect(modelManager, physics, subTileSize));
    effects.push_back((TerrainEffect*)new RoadEffect(subTileSize));
    effects.push_back((TerrainEffect*)new TreeEffect(subTileSize));
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

bool TerrainEffectManager::LoadSubTileEffects(vec::vec2i pos, SubTile* tile)
{
    if (subtileEffectData.find(pos) != subtileEffectData.end())
    {
        // Already in cache.
        return true;
    }

    std::vector<TerrainEffectData*> tileEffects;
    for (auto iter = effects.begin(); iter != effects.end(); iter++)
    {
        void* effectData;
        if ((*iter)->LoadEffect(pos, &effectData, tile))
        {
            tileEffects.push_back(new TerrainEffectData(*iter, effectData));
        }
    }

    subtileEffectData[pos] = tileEffects;
    return true;
}

void TerrainEffectManager::Simulate(const vec::vec2i pos, float elapsedSeconds)
{
    if (subtileEffectData.find(pos) == subtileEffectData.end())
    {
        Logger::LogWarn("Attempted to simulate terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return;
    }

    for (auto iter = subtileEffectData[pos].begin(); iter != subtileEffectData[pos].end(); iter++)
    {
        (*iter)->effect->Simulate(pos, (*iter)->effectData, elapsedSeconds);
    }
}

void TerrainEffectManager::RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (subtileEffectData.find(pos) == subtileEffectData.end())
    {
        Logger::LogWarn("Attempted to render terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return;
    }

    for (auto iter = subtileEffectData[pos].begin(); iter != subtileEffectData[pos].end(); iter++)
    {
        (*iter)->effect->Render((*iter)->effectData, projectionMatrix, mvMatrix);
    }
}

void TerrainEffectManager::CleanupSubTileEffects(vec::vec2i pos, bool log)
{
    for (auto iter = subtileEffectData[pos].begin(); iter != subtileEffectData[pos].end(); iter++)
    {
        (*iter)->effect->UnloadEffect((*iter)->effectData);
        delete (*iter);
    }

    if (log)
    {
        Logger::Log("Unloaded effects ", pos.x, ", ", pos.y, ".");
    }
}

void TerrainEffectManager::UnloadSubTileEffects(vec::vec2i pos)
{
    CleanupSubTileEffects(pos, true);
    subtileEffectData.erase(pos);
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
