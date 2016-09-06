#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "GrassEffect.h"

GrassEffect::GrassEffect(int subTileSize)
    : subTileSize(subTileSize)
{
}

bool GrassEffect::LoadBasics(ShaderManager* shaderManager)
{
    // Load our shader program to custom-render grass.
    if (!shaderManager->CreateShaderProgram("grassRender", &programId))
    {
        Logger::LogError("Failed to load the basic grass rendering shader; cannot continue.");
        return false;
    }

    projMatrixLocation = glGetUniformLocation(programId, "projMatrix");
    mvMatrixLocation = glGetUniformLocation(programId, "mvMatrix");

    return true;
}

bool GrassEffect::LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile)
{
    bool hasGrassEffect = false;
    GrassEffectData* grassEffect = nullptr;

    // Scan the image for grass pixels.
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::GRASSLAND)
            {
                if (!hasGrassEffect)
                {
                    grassEffect = new GrassEffectData();
                    hasGrassEffect = true;
                }
                
                float height = tile->heightmap[i + j * subTileSize];
                vec::vec2i realPos = subtileId * 0.10f + vec::vec2i(i, j);

                // TODO configurable
                vec::vec3 bottomColor = vec::vec3(0.0f, 0.90f + MathOps::Rand() * 0.10f, 0.0f);
                vec::vec3 topColor = vec::vec3(0.0f, 0.50f + MathOps::Rand() * 0.30f, 0.20f + MathOps::Rand() * 0.60f);
                vec::vec3 bottomPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height);
                vec::vec3 topPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height + 0.15f + 1.70f * MathOps::Rand());

                // Add grass
                vec::vec3 lowerOffset = vec::vec3(0.0f);
                vec::vec3 upperOffset = vec::vec3(std::cos(2 * Constants::PI / 2), std::sin(2 * Constants::PI / 2), 0.0f);
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

void GrassEffect::Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds)
{
    // This is still too slow. I need to randomly update not only specific elements, but specific grass segments per subtile.
    // Don't update all the grass at once, that's too slow. Just move a few elements.
    /*int minimizationFactor = 5 + (int)(MathOps::Rand() * 10);
    for (int i = 0; i < effectData[pos]->grassEffect.grassOffsets.size() / 2; i++)
    {
    if (i % minimizationFactor == 0)
    {
    vec::vec3 upperOffset = vec::vec3(MathOps::Rand(), MathOps::Rand(), 0.0f);
    effectData[pos]->grassEffect.grassStalks.positions[i * 2 + 1] =
    (effectData[pos]->grassEffect.grassStalks.positions[i * 2 + 1]
    - effectData[pos]->grassEffect.grassOffsets[i * 2 + 1]) + upperOffset;
    effectData[pos]->grassEffect.grassOffsets[i * 2 + 1] = upperOffset;
    }
    }

    // Modify the grass image to result in a slight waviness of the grass.
    glBindVertexArray(effectData[pos]->grassEffect.vao);
    effectData[pos]->grassEffect.grassStalks.TransferPositionToOpenGl(effectData[pos]->grassEffect.positionBuffer);*/
}

void GrassEffect::Render(void* effectData, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    GrassEffectData* grassEffect = (GrassEffectData*)effectData;

    glUseProgram(programId);
    glBindVertexArray(grassEffect->vao);

    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(mvMatrixLocation, 1, GL_FALSE, mvMatrix);

    glDrawArrays(GL_LINES, 0, grassEffect->grassStalks.positions.size());
}
