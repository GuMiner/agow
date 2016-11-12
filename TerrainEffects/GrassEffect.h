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

struct GrassStats
{
    long stalksRendered;

    long tilesRendered;
    long usRenderTime;

    GrassStats()
    {
        Reset();
    }

    void Reset()
    {
        stalksRendered = 0;
        tilesRendered = 0;

        usRenderTime = 0;
    }
};

class GrassEffect : public TerrainEffect
{
    GLuint programId;

    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;


    static GrassStats stats;
public:
    GrassEffect();
    virtual bool LoadBasics(ShaderFactory* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
    virtual void LogStats() override;
};