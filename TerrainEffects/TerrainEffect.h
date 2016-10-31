#pragma once
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"

// Defines how a terrain effect is operated.
class TerrainEffect
{
public:
    // Loads runtime constants required for this effect.
    virtual bool LoadBasics(ShaderManager* shaderManager) = 0;

    // Loads and unloads an effect into / out of the effect data.
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile) = 0;
    virtual void UnloadEffect(void* effectData) = 0;

    // Simulates an effect.
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) = 0;
    
    // Renders an effect.
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) = 0;
    
    // Logs effect statistics. 
    virtual void LogStats() = 0;
};