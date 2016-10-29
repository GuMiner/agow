#include <glm\gtc\random.hpp>
#include "Utils\Logger.h"
#include "GrassEffect.h"

GrassEffect::GrassEffect()
{
}

bool GrassEffect::LoadBasics(ShaderManager* shaderManager)
{
    // Load our shader program to custom-render grass.
    if (!shaderManager->CreateShaderProgramWithGeometryShader("grassRender", &programId))
    {
        Logger::LogError("Failed to load the basic grass rendering shader; cannot continue.");
        return false;
    }

    projMatrixLocation = glGetUniformLocation(programId, "projMatrix");
    mvMatrixLocation = glGetUniformLocation(programId, "mvMatrix");

    return true;
}

bool GrassEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile)
{
    bool hasGrassEffect = false;
    GrassEffectData* grassEffect = nullptr;

    // Scan the image for grass pixels.
    for (int i = 0; i < TerrainTile::SubtileSize; i++)
    {
        for (int j = 0; j < TerrainTile::SubtileSize; j++)
        {
            int pixelId = tile->GetPixelId(glm::ivec2(i, j));
            if (tile->type[pixelId] == TerrainTypes::GRASSLAND)
            {
                if (!hasGrassEffect)
                {
                    grassEffect = new GrassEffectData();
                    hasGrassEffect = true;
                }
                
                float height = tile->heightmap[pixelId];
                glm::vec2 realPos = TerrainTile::GetRealPosition(subtileId, glm::ivec2(i, j));

                // TODO configurable
                glm::vec3 bottomColor = glm::vec3(0.0f, 0.90f + glm::linearRand(0.0f, 0.10f), 0.0f);
                glm::vec3 topColor = glm::vec3(0.0f, 0.50f + glm::linearRand(0.0f, 0.30f), 0.20f + glm::linearRand(0.0f, 0.60f));
                glm::vec3 bottomPos = glm::vec3((float)realPos.x + glm::linearRand(-0.5f, 0.5f), (float)realPos.y + glm::linearRand(-0.5f, 0.5f), height);
                glm::vec3 topPos = bottomPos + glm::vec3(glm::linearRand(-0.10f, 0.10f), glm::linearRand(-0.10f, 0.10f), 0.15f + 0.50f * glm::linearRand(0.0f, 1.0f));

                // Add grass
                glm::vec3 lowerOffset = glm::vec3(0.0f);
                glm::vec3 upperOffset = glm::vec3(0.0f);
                grassEffect->grassOffsets.push_back(lowerOffset);
                grassEffect->grassOffsets.push_back(upperOffset);

                grassEffect->grassStalks.positions.push_back(bottomPos + lowerOffset);
                grassEffect->grassStalks.positions.push_back(topPos + upperOffset);
                grassEffect->grassStalks.colors.push_back(bottomColor);
                grassEffect->grassStalks.colors.push_back(topColor);
                grassEffect->grassStalks.ids.push_back(grassEffect->grassStalks.positions.size() - 1); // Starts at 1
                grassEffect->grassStalks.ids.push_back(grassEffect->grassStalks.positions.size());
            }
        }
    }

    if (hasGrassEffect)
    {
        // Grass vertex data.
        glGenVertexArrays(1, &grassEffect->vao);
        glBindVertexArray(grassEffect->vao);
        glGenBuffers(1, &grassEffect->positionBuffer);
        glGenBuffers(1, &grassEffect->colorBuffer);

        Logger::Log("Parsed ", grassEffect->grassStalks.positions.size() / 2, " grass stalks.");
        grassEffect->grassStalks.TransferPositionToOpenGl(grassEffect->positionBuffer);
        grassEffect->grassStalks.TransferColorToOpenGl(grassEffect->colorBuffer);
        *effectData = grassEffect;
    }

    return hasGrassEffect;
}

void GrassEffect::UnloadEffect(void* effectData)
{
    GrassEffectData* grassEffect = (GrassEffectData*)effectData;
    glDeleteVertexArrays(1, &grassEffect->vao);
    glDeleteBuffers(1, &grassEffect->positionBuffer);
    glDeleteBuffers(1, &grassEffect->colorBuffer);
    delete grassEffect;
}

void GrassEffect::Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds)
{
    // This is still too slow. I need to randomly update not only specific elements, but specific grass segments per subtile.
    // Don't update all the grass at once, that's too slow. Just move a few elements.
    /*int minimizationFactor = 5 + (int)(MathOps::Rand() * 10);
    for (int i = 0; i < effectData[start]->grassEffect.grassOffsets.size() / 2; i++)
    {
    if (i % minimizationFactor == 0)
    {
    glm::vec3 upperOffset = glm::vec3(MathOps::Rand(), MathOps::Rand(), 0.0f);
    effectData[start]->grassEffect.grassStalks.positions[i * 2 + 1] =
    (effectData[start]->grassEffect.grassStalks.positions[i * 2 + 1]
    - effectData[start]->grassEffect.grassOffsets[i * 2 + 1]) + upperOffset;
    effectData[start]->grassEffect.grassOffsets[i * 2 + 1] = upperOffset;
    }
    }

    // Modify the grass image to result in a slight waviness of the grass.
    glBindVertexArray(effectData[start]->grassEffect.vao);
    effectData[start]->grassEffect.grassStalks.TransferPositionToOpenGl(effectData[start]->grassEffect.positionBuffer);*/
}

void GrassEffect::Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    GrassEffectData* grassEffect = (GrassEffectData*)effectData;

    glLineWidth(3.0f);
    glUseProgram(programId);
    glBindVertexArray(grassEffect->vao);

    glm::mat4 viewModelMatrix = viewMatrix * modelMatrix;
    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, &perspectiveMatrix[0][0]);
    glUniformMatrix4fv(mvMatrixLocation, 1, GL_FALSE, &viewModelMatrix[0][0]);

    glDrawArrays(GL_LINES, 0, grassEffect->grassStalks.positions.size());
    glLineWidth(1.0f);
}
