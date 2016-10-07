#include <glm\gtc\random.hpp>
#include "Utils\Logger.h"
#include "TreeEffect.h"

TreeEffect::TreeEffect(const std::string& cacheFolder, int subTileSize)
    : treeCache(cacheFolder, "trees"), subTileSize(subTileSize)
{
}

bool TreeEffect::LoadBasics(ShaderManager* shaderManager)
{
    // Load our shader programs to custom-render trunks and leaves.
    if (!shaderManager->CreateShaderProgram("treeTrunkRender", &trunkProgram.programId))
    {
        Logger::LogError("Failed to load the tree trunk rendering shader; cannot continue.");
        return false;
    }

    trunkProgram.projMatrixLocation = glGetUniformLocation(trunkProgram.programId, "projMatrix");
    trunkProgram.mvMatrixLocation = glGetUniformLocation(trunkProgram.programId, "mvMatrix");

    if (!shaderManager->CreateShaderProgram("treeLeafRender", &leafProgram.programId))
    {
        Logger::LogError("Failed to load the tree leaf rendering shader; cannot continue.");
        return false;
    }

    leafProgram.projMatrixLocation = glGetUniformLocation(leafProgram.programId, "projMatrix");
    leafProgram.mvMatrixLocation = glGetUniformLocation(leafProgram.programId, "mvMatrix");

    return true;
}

bool TreeEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile)
{
    bool hasTreeEffect = false;
    TreeEffectData* treeEffect = nullptr;

    bool isInCache = treeCache.IsInCache(subtileId);
    TreeCacheData treeCacheData;
    if (isInCache)
    {
        // TODO this can also mean 'skip tree effect'.
        TreeCacheData* pointer = &treeCacheData;
        treeCache.LoadFromCache(subtileId, (void**)&pointer);
        hasTreeEffect = treeCacheData.hasEffect;
        if (hasTreeEffect)
        {
            treeEffect = new TreeEffectData();
            treeEffect->treeTrunks.vertices.positions = std::move(treeCacheData.branches);
            treeEffect->treeTrunks.vertices.colors = std::move(treeCacheData.branchColors);

            treeEffect->treeLeaves.vertices.positions = std::move(treeCacheData.leaves);
            treeEffect->treeLeaves.vertices.colors = std::move(treeCacheData.leafColors);
        }
    }
    else
    {
        // Scan the image for tree pixels.
        for (int i = 0; i < subTileSize; i++)
        {
            for (int j = 0; j < subTileSize; j++)
            {
                // TOOD configurable density
                if (tile->type[i + j * subTileSize] == TerrainTypes::TREES && glm::linearRand(0.0f, 1.0f) > 0.90f)
                {
                    if (!hasTreeEffect)
                    {
                        treeEffect = new TreeEffectData();
                        hasTreeEffect = true;
                    }

                    float height = tile->heightmap[i + j * subTileSize];
                    glm::ivec2 realPos = subtileId / 10 + glm::ivec2(i, j);

                    // TODO configurable
                    glm::vec3 bottomColor = glm::vec3(0.57f, 0.20f + glm::linearRand(0.0f, 0.10f), 0.10f);
                    glm::vec3 topColor = glm::vec3(0.57f, 0.20f + glm::linearRand(0.0f, 0.30f), 0.10f + glm::linearRand(0.0f, 0.40f));
                    glm::vec3 bottomPos = glm::vec3((float)realPos.x + glm::linearRand(0.0f, 2.0f) - 1.0f, (float)realPos.y + glm::linearRand(0.0f, 2.0f) - 1.0f, height);
                    glm::vec3 topPos = bottomPos + glm::vec3(0, 0, 1.0f);

                     GenerationResults results = treeGenerator.GenerateTree(bottomPos,
                         &treeEffect->treeTrunks.vertices.positions, nullptr,
                         &treeEffect->treeLeaves.vertices.positions);
                    for (unsigned int i = 0; i < results.branches; i++) 
                    {
                        // Add tree trunk colors;
                        treeEffect->treeTrunks.vertices.colors.push_back(bottomColor);
                        treeEffect->treeTrunks.vertices.colors.push_back(bottomColor);
                    }

                    // Add tree leaf colors.
                    for (unsigned int i = 0; i < results.leaves; i++) // 
                    {
                        treeEffect->treeLeaves.vertices.colors.push_back(glm::vec3(0.1f, 0.70f + glm::linearRand(0.0f, 0.30f), 0.0f));
                    }
                }
            }
        }

        // Save our tree data to the cache to speed up loading the next time around.
        TreeCacheInputData inputData(hasTreeEffect,
            hasTreeEffect ? &(treeEffect->treeTrunks.vertices.positions) : nullptr,
            hasTreeEffect ? &(treeEffect->treeTrunks.vertices.colors) : nullptr,
            hasTreeEffect ? &(treeEffect->treeLeaves.vertices.positions) : nullptr,
            hasTreeEffect ? &(treeEffect->treeLeaves.vertices.colors) : nullptr);

        treeCache.SaveToCache(subtileId, &inputData);
    }

    if (hasTreeEffect)
    {
        // Tree trunk and leave vertex data.
        glGenVertexArrays(1, &treeEffect->treeTrunks.vao);
        glBindVertexArray(treeEffect->treeTrunks.vao);
        glGenBuffers(1, &treeEffect->treeTrunks.positionBuffer);
        glGenBuffers(1, &treeEffect->treeTrunks.colorBuffer);

        Logger::Log("Parsed ", treeEffect->treeTrunks.vertices.positions.size() / 2, " tree trunks.");
        treeEffect->treeTrunks.vertices.TransferPositionToOpenGl(treeEffect->treeTrunks.positionBuffer);
        treeEffect->treeTrunks.vertices.TransferColorToOpenGl(treeEffect->treeTrunks.colorBuffer);

        glGenVertexArrays(1, &treeEffect->treeLeaves.vao);
        glBindVertexArray(treeEffect->treeLeaves.vao);
        glGenBuffers(1, &treeEffect->treeLeaves.positionBuffer);
        glGenBuffers(1, &treeEffect->treeLeaves.colorBuffer);

        Logger::Log("Parsed ", treeEffect->treeLeaves.vertices.positions.size() / 2, " tree leaves.");
        treeEffect->treeLeaves.vertices.TransferPositionToOpenGl(treeEffect->treeLeaves.positionBuffer);
        treeEffect->treeLeaves.vertices.TransferColorToOpenGl(treeEffect->treeLeaves.colorBuffer);

        *effectData = treeEffect;
    }

    return hasTreeEffect;
}

void TreeEffect::UnloadEffect(void* effectData)
{
    TreeEffectData* treeEffect = (TreeEffectData*)effectData;
    glDeleteVertexArrays(1, &treeEffect->treeTrunks.vao);
    glDeleteBuffers(1, &treeEffect->treeTrunks.positionBuffer);
    glDeleteBuffers(1, &treeEffect->treeTrunks.colorBuffer);

    glDeleteVertexArrays(1, &treeEffect->treeLeaves.vao);
    glDeleteBuffers(1, &treeEffect->treeLeaves.positionBuffer);
    glDeleteBuffers(1, &treeEffect->treeLeaves.colorBuffer);

    delete treeEffect;
}

void TreeEffect::Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds)
{
    // TODO wave the trees slightly over time.
}

void TreeEffect::Render(void* effectData, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    TreeEffectData* treeEffect = (TreeEffectData*)effectData;

    // Render trunks.
    glLineWidth(2.0f);
    glUseProgram(trunkProgram.programId);
    glBindVertexArray(treeEffect->treeTrunks.vao);

    glUniformMatrix4fv(trunkProgram.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(trunkProgram.mvMatrixLocation, 1, GL_FALSE, &(viewMatrix * modelMatrix)[0][0]);

    glDrawArrays(GL_LINES, 0, treeEffect->treeTrunks.vertices.positions.size());
    glLineWidth(1.0f);

    // Render leaves.
    glUseProgram(leafProgram.programId);
    glBindVertexArray(treeEffect->treeLeaves.vao);

    glUniformMatrix4fv(leafProgram.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(leafProgram.mvMatrixLocation, 1, GL_FALSE, &(viewMatrix * modelMatrix)[0][0]);

    glDrawArrays(GL_POINTS, 0, treeEffect->treeLeaves.vertices.positions.size());
}
