#pragma once
#include "Cache\TreeCache.h"
#include "Utils\Vertex.h"
#include "TerrainEffect.h"
#include "TreeGenerator.h"

struct VertexData
{
    GLuint vao;
    GLuint positionBuffer;
    GLuint colorBuffer;

    universalVertices vertices;
};

struct TreeEffectData
{
    VertexData treeTrunks;
    VertexData treeLeaves;
};

struct TreeProgram
{
    GLuint programId;

    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;
};

class TreeEffect : public TerrainEffect
{
    int subTileSize; // In pixels

    TreeCache treeCache;

    TreeProgram trunkProgram;
    TreeProgram leafProgram;
    TreeGenerator treeGenerator;

public:
    TreeEffect(const std::string& cacheFolder, int subTileSize);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(vec::vec2i subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix) override;
};