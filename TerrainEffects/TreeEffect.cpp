#include <glm\mat4x4.hpp>
#include <glm\gtc\random.hpp>
#include <SFML\System.hpp>
#include <algorithm>
#include "Generators\ColorGenerator.h"
#include "Managers\TerrainManager.h"
#include "logging\Logger.h"
#include "TreeEffect.h"

TreeStats TreeEffect::stats = TreeStats();

TreeEffect::TreeEffect(const std::string& cacheFolder)
    : treeCache(cacheFolder, "trees")
{
}

bool TreeEffect::LoadBasics(ShaderFactory* shaderManager)
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

    // TODO configurable number of trees we generate.
    for (int i = 0; i < 100; i++)
    {
        TreeCacheData generatedTree;
        glm::ivec2 treeId = glm::ivec2(0, i);
        if (treeCache.IsInCache(treeId))
        {
            TreeCacheData* pointer = &generatedTree;
            treeCache.LoadFromCache(treeId, (void**)&pointer);
        }
        else
        {
            // Generate the possible tree models to use for rendering trees.
            GenerationResults results = treeGenerator.GenerateTree(&generatedTree.branches, &generatedTree.branchThicknesses, &generatedTree.leaves);
            for (unsigned int i = 0; i < results.branches; i++)
            {
                // Add tree trunk colors;
                generatedTree.branchColors.push_back(ColorGenerator::GetTreeBranchColor());
                generatedTree.branchColors.push_back(ColorGenerator::GetTreeBranchColor());
            }

            // Add tree leaf colors.
            for (unsigned int i = 0; i < results.leaves; i++)
            {
                generatedTree.leafColors.push_back(ColorGenerator::GetTreeLeafColor());
            }

            // Save to cache so we don't need to generate it next time.
            TreeCacheInputData inputData(&generatedTree.branches, &generatedTree.branchColors, &generatedTree.branchThicknesses, 
                &generatedTree.leaves, &generatedTree.leafColors);

            treeCache.SaveToCache(treeId, &inputData);
        }

        cachedTrees.push_back(generatedTree);
    }

    return true;
}

bool TreeEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile)
{
    bool hasTreeEffect = false;
    TreeEffectData* treeEffect = nullptr;

    // TODO do we want to cache from the cached trees?
    // Scan the image for tree pixels.
    int treesInRegion = 0;
    for (int i = 0; i < TerrainTile::SubtileSize; i++)
    {
        for (int j = 0; j < TerrainTile::SubtileSize; j++)
        {
            // TOOD configurable density
            int pixelId = tile->GetPixelId(glm::ivec2(i, j));
            if (tile->type[pixelId] == TerrainTypes::TREES && glm::linearRand(0.0f, 1.0f) > 0.90f)
            {
                ++treesInRegion;
                if (!hasTreeEffect)
                {
                    treeEffect = new TreeEffectData();
                    hasTreeEffect = true;
                }

                float height = tile->heightmap[i + j * TerrainTile::SubtileSize];
                glm::vec3 bottomPos = glm::vec3((float)i + glm::linearRand(-1.0f, 1.0f), (float)j + glm::linearRand(-1.0f, 1.0f), height);

                // Copy over a cached tree into this location.
                const TreeCacheData& tree = cachedTrees[glm::linearRand(0, (int)(cachedTrees.size() - 1))];

                // Append the vectors that can be appended.
                treeEffect->treeTrunks.vertices.colors.insert(treeEffect->treeTrunks.vertices.colors.begin(), tree.branchColors.begin(), tree.branchColors.end());
                treeEffect->treeTrunks.vertices.ids.insert(treeEffect->treeTrunks.vertices.ids.begin(), tree.branchThicknesses.begin(), tree.branchThicknesses.end());
                treeEffect->treeLeaves.vertices.colors.insert(treeEffect->treeLeaves.vertices.colors.begin(), tree.leafColors.begin(), tree.leafColors.end());
                
                // Translate those elements that cannot be.
                for (unsigned int i = 0; i < tree.branches.size(); i++)
                {
                    treeEffect->treeTrunks.vertices.positions.push_back(tree.branches[i] + bottomPos);
                }

                for (unsigned int i = 0; i < tree.leaves.size(); i++)
                {
                    treeEffect->treeLeaves.vertices.positions.push_back(tree.leaves[i] + bottomPos);
                }
            }
        }
    }

    if (hasTreeEffect)
    {
        Logger::Log("Parsed ", treesInRegion, " trees in [", subtileId.x, ", ", subtileId.y, "].");

        // Tree trunk and leave vertex data.
        glGenVertexArrays(1, &treeEffect->treeTrunks.vao);
        glBindVertexArray(treeEffect->treeTrunks.vao);
        glGenBuffers(1, &treeEffect->treeTrunks.positionBuffer);
        glGenBuffers(1, &treeEffect->treeTrunks.colorBuffer);
        glGenBuffers(1, &treeEffect->treeTrunks.idBuffer);

        Logger::Log("Parsed ", treeEffect->treeTrunks.vertices.positions.size() / 2, " tree trunks.");
        treeEffect->treeTrunks.vertices.TransferPositionToOpenGl(treeEffect->treeTrunks.positionBuffer);
        treeEffect->treeTrunks.vertices.TransferIdsToOpenGl(treeEffect->treeTrunks.idBuffer);
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
    glDeleteBuffers(1, &treeEffect->treeTrunks.idBuffer);

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
    sf::Clock clock;
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

    stats.usRenderTime += (long)clock.getElapsedTime().asMicroseconds();
    stats.trunksRendered += treeEffect->treeTrunks.vertices.positions.size() / 2;
    stats.leavesRendered += treeEffect->treeLeaves.vertices.positions.size();
    stats.tilesRendered++;
}

void TreeEffect::LogStats()
{
    Logger::Log("Tree Rendering: ", stats.usRenderTime, " us, ", stats.trunksRendered, " trunks, ", stats.leavesRendered, " leaves, ", stats.tilesRendered, " tiles.");
    stats.Reset();
}
