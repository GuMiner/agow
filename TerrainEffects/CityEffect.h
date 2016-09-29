#pragma once
#include "Cache\TreeCache.h"
#include "Utils\Vertex.h"
#pragma once
#include "TerrainEffect.h"

struct CityEffectData
{

};

class CityEffect : public TerrainEffect
{
    int subTileSize; // In pixels



public:
    CityEffect(const std::string& cacheFolder, int subTileSize);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix) override;
};