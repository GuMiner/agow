#pragma once
#include <vector>
#include "Data\Model.h"
#include "Data\UserPhysics.h"
#include "Managers\ModelManager.h"
#include "Utils\TypedCallback.h"
#include "Physics.h"
#include "TerrainEffect.h"

struct Building
{
    bool separated;
    glm::vec4 color;
    std::vector<Model> segments;
};

struct CityEffectData
{
    std::vector<Building> buildings;
    bool isHighDensity;
};

struct BuildingCollisionCallbackData
{
    CityEffectData* effect;
    int buildingId;
};

struct CityStats
{
    long segmentsRendered;

    long tilesRendered;
    long usRenderTime;

    CityStats()
    {
        Reset();
    }

    void Reset()
    {
        segmentsRendered = 0;
        tilesRendered = 0;

        usRenderTime = 0;
    }
};

class CityEffect : public TerrainEffect, ICallback<UserPhysics::ObjectType>
{
    ModelManager* modelManager;
    Physics* physics;

    static CityStats stats;

public:
    CityEffect(ModelManager* modelManager, Physics* physics, const std::string& cacheFolder);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;

    // Handles building collisions.
    virtual void Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData) override;
    virtual void LogStats() override;
};