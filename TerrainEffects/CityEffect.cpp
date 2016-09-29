#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "BuildingGenerator.h"
#include "CityEffect.h"

CityEffect::CityEffect(const std::string& cacheFolder, int subTileSize)
    : subTileSize(subTileSize) // TODO use the cache to avoid regeneraitng buildings.
{
}

bool CityEffect::LoadBasics(ShaderManager* shaderManager)
{
    // TODO
    return true;
}

bool CityEffect::LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile)
{
    BuildingGenerator buildingGenerator;

    // TODO
    return false;
}

void CityEffect::UnloadEffect(void* effectData)
{
    CityEffectData* CityEffect = (CityEffectData*)effectData;
    // TODO

    delete CityEffect;
}

void CityEffect::Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds)
{
}

void CityEffect::Render(void* effectData, const vec::mat4& projectionMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix)
{
    CityEffectData* CityEffect = (CityEffectData*)effectData;
    // TODO
}
