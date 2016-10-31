#pragma once
#include "Cache\TreeCache.h"
#include "Generators\TreeGenerator.h"
#include "Utils\Vertex.h"
#include "TerrainEffect.h"

struct VertexData
{
    GLuint vao;
    GLuint positionBuffer;
    GLuint colorBuffer;
    GLuint idBuffer;

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

struct TreeStats
{
    long trunksRendered;
    long leavesRendered;

    long tilesRendered;
    long usRenderTime;

    TreeStats()
    {
        Reset();
    }

    void Reset()
    {
        trunksRendered = 0;
        leavesRendered = 0;
        tilesRendered = 0;

        usRenderTime = 0;
    }
};

class TreeEffect : public TerrainEffect
{
    TreeCache treeCache;
    std::vector<TreeCacheData> cachedTrees;

    TreeProgram trunkProgram;
    TreeProgram leafProgram;
    TreeGenerator treeGenerator;

    static TreeStats stats;

public:
    TreeEffect(const std::string& cacheFolder);
    virtual bool LoadBasics(ShaderManager* shaderManager) override;
    virtual bool LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile) override;
    virtual void UnloadEffect(void* effectData) override;
    virtual void Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds) override;
    virtual void Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) override;
    virtual void LogStats() override;
};