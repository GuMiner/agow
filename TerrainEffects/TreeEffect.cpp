#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "TreeEffect.h"

TreeEffect::TreeEffect(int subTileSize)
    : subTileSize(subTileSize)
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

    GenerationResults results = treeGenerator.GenerateTree(TreeType::SPHERE_DENSE, vec::vec3(0, 0, 0), 2, 4,
        &branches,
        nullptr,
        &leaves);

    return true;
}

bool TreeEffect::LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile)
{
    bool hasTreeEffect = false;
    TreeEffectData* treeEffect = nullptr;

    // Scan the image for tree pixels.
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            // TOOD configurable density
            if (tile->type[i + j * subTileSize] == TerrainTypes::TREES && MathOps::Rand() > 0.90f)
            {
                if (!hasTreeEffect)
                {
                    treeEffect = new TreeEffectData();
                    hasTreeEffect = true;
                }
                
                float height = tile->heightmap[i + j * subTileSize];
                vec::vec2i realPos = subtileId * 0.10f + vec::vec2i(i, j);

                // TODO configurable
                vec::vec3 bottomColor = vec::vec3(0.57f, 0.20f + MathOps::Rand() * 0.10f, 0.10f);
                vec::vec3 topColor = vec::vec3(0.57f, 0.20f + MathOps::Rand() * 0.30f, 0.10f + MathOps::Rand() * 0.40f);
                vec::vec3 bottomPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height);
                vec::vec3 topPos = bottomPos + vec::vec3(0, 0, 1.0f);

                // GenerationResults results = treeGenerator.GenerateTree(bottomPos,
                //     &treeEffect->treeTrunks.vertices.positions, nullptr,
                //     &treeEffect->treeLeaves.vertices.positions);
                for (unsigned int i = 0; i < branches.size() / 2; i++) // results.branches
                {
                    // Add tree trunks.
                    treeEffect->treeTrunks.vertices.positions.push_back(branches[i * 2] + bottomPos);
                    treeEffect->treeTrunks.vertices.positions.push_back(branches[i * 2 + 1] + bottomPos);
                    treeEffect->treeTrunks.vertices.colors.push_back(bottomColor);
                    treeEffect->treeTrunks.vertices.colors.push_back(bottomColor);
                }

                // Add tree leaves.
                for (unsigned int i = 0; i <  leaves.size(); i++) // results.leaves
                {
                    treeEffect->treeLeaves.vertices.positions.push_back(leaves[i] + bottomPos);
                    treeEffect->treeLeaves.vertices.colors.push_back(vec::vec3(0.1f, 0.70f + MathOps::Rand() * 0.30f, 0.0f));
                }
            }
        }
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

void TreeEffect::Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds)
{
    // TODO wave the trees slightly over time.
}

void TreeEffect::Render(void* effectData, const vec::mat4& projectionMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix)
{
    TreeEffectData* treeEffect = (TreeEffectData*)effectData;

    // Render trunks.
    glLineWidth(3.0f);
    glUseProgram(trunkProgram.programId);
    glBindVertexArray(treeEffect->treeTrunks.vao);

    glUniformMatrix4fv(trunkProgram.projMatrixLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(trunkProgram.mvMatrixLocation, 1, GL_FALSE, viewMatrix * modelMatrix);

    glDrawArrays(GL_LINES, 0, treeEffect->treeTrunks.vertices.positions.size());
    glLineWidth(1.0f);

    // Render leaves.
    glUseProgram(leafProgram.programId);
    glBindVertexArray(treeEffect->treeLeaves.vao);

    glUniformMatrix4fv(leafProgram.projMatrixLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(leafProgram.mvMatrixLocation, 1, GL_FALSE, viewMatrix * modelMatrix);

    glDrawArrays(GL_POINTS, 0, treeEffect->treeLeaves.vertices.positions.size());
}
