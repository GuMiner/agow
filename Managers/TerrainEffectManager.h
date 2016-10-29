#pragma once
#include <string>
#include <set>
#include <map>
#include <GL/glew.h>
#include "Data\Model.h"
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Managers\ModelManager.h"
#include <glm\vec3.hpp>
#include "TerrainEffects\TerrainEffect.h"
#include "Utils\Vertex.h"
#include "BasicPhysics.h"

struct TerrainEffectData
{
    TerrainEffect* effect;
    void* effectData;

    TerrainEffectData(TerrainEffect* effect, void* effectData)
        : effect(effect), effectData(effectData)
    {
    }
};

// Defines effects for each sub tile of terrain.
class TerrainEffectManager
{
    ShaderManager* shaderManager;
    ModelManager* modelManager;
    BasicPhysics* physics;

    std::vector<TerrainEffect*> effects;
    std::map<glm::ivec2, std::vector<TerrainEffectData*>, iVec2Comparer> subtileEffectData;

    void CleanupSubTileEffects(glm::ivec2 start, bool log);

public:
    TerrainEffectManager(ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* basicPhysics);

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadSubTileEffects(glm::ivec2 start, SubTile* tile);
    
    // Simulates the effects for the loaded tile.
    void Simulate(const glm::ivec2 start, float elapsedSeconds);

    // Renders a tile's effects *The tile must have been loaded ahead-of-time.*
    void RenderSubTileEffects(const glm::ivec2 start, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix);

    void UnloadSubTileEffects(glm::ivec2 start);
    virtual ~TerrainEffectManager();
};

