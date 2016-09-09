#pragma once
#include "Utils\Vertex.h"
#include "TerrainEffect.h"

struct GrassEffectData
{
    GLuint vao;
    GLuint positionBuffer;
    GLuint colorBuffer;

    universalVertices grassStalks;
    std::vector<vec::vec3> grassOffsets;
};

class GrassEffect : public TerrainEffect
{
    int subTileSize; // In pixels

    GLuint programId;

    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;

public:
    GrassEffect(int subTileSize);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix) override;
};