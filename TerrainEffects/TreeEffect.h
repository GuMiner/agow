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
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile * tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
};