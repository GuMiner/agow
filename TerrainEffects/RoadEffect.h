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
    std::vector<vec::vec2i> roadPositions;

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

    bool GetNearbyCheckOne(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
    bool GetNearbyCheckTwo(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
    bool GetNearbyCheckThree(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
    bool GetNearbyCheckFour(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
    vec::vec2i GetNearbyType(SubTile* tile, const vec::vec2i pos, const int terrainType) const;

public:
    RoadEffect(int subTileSize);

    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix) override;
};