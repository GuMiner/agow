#pragma once
#include <string>
#include <set>
#include <map>
#include <GL/glew.h>
#include "Data\Model.h"
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Managers\ModelManager.h"
#include "Math\Vec.h"
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

    int subTileSize; // In pixels

    std::vector<TerrainEffect*> effects;
    std::map<vec::vec2i, std::vector<TerrainEffectData*>, vec::vec2iComparer> subtileEffectData;

    void CleanupSubTileEffects(vec::vec2i start, bool log);

public:
    TerrainEffectManager(ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* basicPhysics, int subTileSize);

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadSubTileEffects(vec::vec2i start, SubTile* tile);
    
    // Simulates the effects for the loaded tile.
    void Simulate(const vec::vec2i start, float elapsedSeconds);

    // Renders a tile's effects *The tile must have been loaded ahead-of-time.*
    void RenderSubTileEffects(const vec::vec2i start, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix);

    void UnloadSubTileEffects(vec::vec2i start);
    virtual ~TerrainEffectManager();
};

