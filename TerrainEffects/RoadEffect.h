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
    std::vector<vec::vec2> positions;
    std::vector<vec::vec2> velocities;

    RoadEffectData(SubTile* tile)
        : tile(tile)
    {
    }
};

class RoadEffect : public TerrainEffect
{
    int subTileSize; // In pixels

    GLuint programId;

    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;

    // Moves the specified traveller, returning the height of the ground the traveller is now above.
    float MoveTraveller(const vec::vec2i subtileId, RoadEffectData* roadEffect, int travellerId, float elapsedSeconds);

public:
    RoadEffect(int subTileSize);

    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix) override;
};