#pragma once
#include "Utils\Vertex.h"
#include "TerrainEffect.h"

struct GrassEffectData
{
    GLuint vao;
    GLuint positionBuffer;
    GLuint colorBuffer;

    universalVertices grassStalks;
    std::vector<glm::vec3> grassOffsets;
};

class GrassEffect : public TerrainEffect
{
    int subTileSize; // In pixels

    GLuint programId;

    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;

public:
    GrassEffect();
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
};