#pragma once
#include <vector>
#include "Data\Model.h"
#include "Managers\ModelManager.h"
#include "BasicPhysics.h"
#include "TerrainEffect.h"

struct Building
{
    glm::vec4 color;
    std::vector<ScaledPhysicalModel> segments;
};

struct CityEffectData
{
    std::vector<Building> buildings;
};

class CityEffect : public TerrainEffect
{
    int subTileSize; // In pixels

    ModelManager* modelManager;
    BasicPhysics* physics;

public:
    CityEffect(ModelManager* modelManager, BasicPhysics* physics, const std::string& cacheFolder, int subTileSize);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
};