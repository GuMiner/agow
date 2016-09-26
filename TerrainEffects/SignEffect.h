#pragma once
#include "Data\Model.h"
#include "Managers\ModelManager.h"
#include "BasicPhysics.h"
#include "TerrainEffect.h"

struct SignEffectData
{
    std::vector<ColoredPhysicalModel> signs;
};

class SignEffect : public TerrainEffect
{
    ModelManager* modelManager;
    BasicPhysics* physics;
    int subTileSize; // In pixels

public:
    SignEffect(ModelManager* modelManager, BasicPhysics* physics, int subTileSize);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix) override;
};