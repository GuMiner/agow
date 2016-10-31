#pragma once
#include "Data\Model.h"
#include "Managers\ModelManager.h"
#include "Physics.h"
#include "TerrainEffect.h"

struct RockEffectData
{
    std::vector<Model> rocks;
};

class RockEffect : public TerrainEffect
{
    ModelManager* modelManager;
    Physics* physics;

public:
    RockEffect(ModelManager* modelManager, Physics* physics);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
    virtual void LogStats() override;
};