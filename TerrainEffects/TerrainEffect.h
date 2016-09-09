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
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile) = 0;
    virtual void UnloadEffect(void* effectData) = 0;

    // Simulates an effect.
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) = 0;
    
    // Renders an effect.
    virtual void Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix) = 0;
};