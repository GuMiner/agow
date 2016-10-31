#pragma once
#include "Utils\Vertex.h"
#include "TerrainEffect.h"

struct RoadEffectData
{
    SubTile* tile;

    GLuint vao;
    GLuint positionBuffer;
    GLuint colorBuffer;

    universalVertices travellers;
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;

    RoadEffectData(SubTile* tile)
        : tile(tile)
    {
    }
};

struct RoadStats
{
    long travellersRendered;

    long tilesRendered;
    long usRenderTime;

    RoadStats()
    {
        Reset();
    }

    void Reset()
    {
        travellersRendered = 0;
        tilesRendered = 0;

        usRenderTime = 0;
    }
};

class RoadEffect : public TerrainEffect
{
    GLuint programId;

    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;

    static RoadStats stats;

    // Moves the specified traveller, returning the height of the ground the traveller is now above.
    float MoveTraveller(const glm::ivec2 subtileId, RoadEffectData* roadEffect, int travellerId, float elapsedSeconds);

public:
    RoadEffect();

    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
    virtual void LogStats() override;
};