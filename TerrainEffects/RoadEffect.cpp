#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "RoadEffect.h"

RoadEffect::RoadEffect(int subTileSize)
    : subTileSize(subTileSize)
{
}

bool RoadEffect::LoadBasics(ShaderManager* shaderManager)
{
    if (!shaderManager->CreateShaderProgram("roadRender", &programId))
    {
        Logger::LogError("Failed to load the road rendering shader; cannot continue.");
        return false;
    }

    projMatrixLocation = glGetUniformLocation(programId, "projMatrix");
    mvMatrixLocation = glGetUniformLocation(programId, "mvMatrix");

    return true;
}

bool RoadEffect::GetNearbyCheckOne(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
{
    if (pos.x != 0)
    {
        if (tile->type[(pos.x - 1) + pos.y * subTileSize] == terrainType)
        {
            *result = pos + vec::vec2i(-1, 0);
            return true;
        }
    }

    return false;
}

bool RoadEffect::GetNearbyCheckTwo(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
{
    if (pos.y != 0)
    {
        if (tile->type[pos.x + (pos.y - 1) * subTileSize] == terrainType)
        {
            *result = pos + vec::vec2i(0, -1);
            return true;
        }
    }

    return false;
}

bool RoadEffect::GetNearbyCheckThree(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
{
    if (pos.x != subTileSize - 1)
    {
        if (tile->type[(pos.x + 1) + pos.y * subTileSize] == terrainType)
        {
            *result = pos + vec::vec2i(1, 0);
            return true;
        }
    }

    return false;
}

bool RoadEffect::GetNearbyCheckFour(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
{
    if (pos.y != subTileSize - 1)
    {
        if (tile->type[pos.x + (pos.y + 1) * subTileSize] == terrainType)
        {
            *result = pos + vec::vec2i(0, 1);
            return true;
        }
    }

    return false;
}

vec::vec2i RoadEffect::GetNearbyType(SubTile* tile, const vec::vec2i pos, const int terrainType) const
{
    vec::vec2i result(-1, -1);
    bool foundNearbyType = false;

    int checkOrder = MathOps::Rand(0, 8);

    switch (checkOrder)
    {
    case 0:
        if (GetNearbyCheckOne(tile, pos, terrainType, &result) || GetNearbyCheckTwo(tile, pos, terrainType, &result)
            || GetNearbyCheckThree(tile, pos, terrainType, &result) || GetNearbyCheckFour(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 1:
        if (GetNearbyCheckFour(tile, pos, terrainType, &result) || GetNearbyCheckThree(tile, pos, terrainType, &result)
            || GetNearbyCheckTwo(tile, pos, terrainType, &result) || GetNearbyCheckOne(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 2:
        if (GetNearbyCheckTwo(tile, pos, terrainType, &result) || GetNearbyCheckOne(tile, pos, terrainType, &result)
            || GetNearbyCheckFour(tile, pos, terrainType, &result) || GetNearbyCheckThree(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 3:
        if (GetNearbyCheckTwo(tile, pos, terrainType, &result) || GetNearbyCheckThree(tile, pos, terrainType, &result)
            || GetNearbyCheckOne(tile, pos, terrainType, &result) || GetNearbyCheckFour(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 4:
        if (GetNearbyCheckFour(tile, pos, terrainType, &result) || GetNearbyCheckOne(tile, pos, terrainType, &result)
            || GetNearbyCheckThree(tile, pos, terrainType, &result) || GetNearbyCheckTwo(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 5:
        if (GetNearbyCheckTwo(tile, pos, terrainType, &result) || GetNearbyCheckThree(tile, pos, terrainType, &result)
            || GetNearbyCheckFour(tile, pos, terrainType, &result) || GetNearbyCheckOne(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 6:
        if (GetNearbyCheckThree(tile, pos, terrainType, &result) || GetNearbyCheckFour(tile, pos, terrainType, &result)
            || GetNearbyCheckOne(tile, pos, terrainType, &result) || GetNearbyCheckTwo(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    case 7:
        if (GetNearbyCheckFour(tile, pos, terrainType, &result) || GetNearbyCheckOne(tile, pos, terrainType, &result)
            || GetNearbyCheckTwo(tile, pos, terrainType, &result) || GetNearbyCheckThree(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    default:
        if (GetNearbyCheckThree(tile, pos, terrainType, &result) || GetNearbyCheckTwo(tile, pos, terrainType, &result)
            || GetNearbyCheckOne(tile, pos, terrainType, &result) || GetNearbyCheckFour(tile, pos, terrainType, &result))
        {
            break;
        }

        break;
    }


    return result;
}

bool RoadEffect::LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile)
{
    bool hasRoadEffect = false;
    RoadEffectData* roadEffect = nullptr;

    // Scan the image for road pixels.
    int roadCounter = 1;
    const long ROAD_SUBCOUNT = 10;
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::ROADS)
            {
                ++roadCounter;

                if (roadCounter % ROAD_SUBCOUNT == 0)
                {
                    vec::vec2i nearbyRoad = GetNearbyType(tile, vec::vec2i(i, j), TerrainTypes::ROADS);
                    if (nearbyRoad.x != -1)
                    {
                        if (!hasRoadEffect)
                        {
                            hasRoadEffect = true;
                            roadEffect = new RoadEffectData(tile);
                        }

                        float height = tile->heightmap[i + j * subTileSize];
                        float height2 = tile->heightmap[nearbyRoad.x + nearbyRoad.y * subTileSize];
                        vec::vec2i realLastPos = subtileId * 0.10f + nearbyRoad;
                        vec::vec2i realPos = subtileId * 0.10f + vec::vec2i(i, j);

                        // TODO configurable
                        vec::vec3 bottomColor = vec::vec3(0.0f, 0.50f, MathOps::Rand() * 0.20f + 0.80f);
                        vec::vec3 topColor = vec::vec3(0.40f + 0.60f * MathOps::Rand(), 0.0f, 0.20f + MathOps::Rand() * 0.40f);
                        vec::vec3 bottomPos = vec::vec3((float)realPos.x, (float)realPos.y, height + 0.5f);
                        vec::vec3 topPos = vec::vec3((float)realLastPos.x, (float)realLastPos.y, height2 + 0.5f);

                        // Add grass
                        roadEffect->travellers.positions.push_back(bottomPos);
                        roadEffect->travellers.positions.push_back(topPos);
                        roadEffect->travellers.colors.push_back(bottomColor);
                        roadEffect->travellers.colors.push_back(topColor);
                        roadEffect->roadPositions.push_back(realPos);
                    }
                }
            }
        }
    }

    if (hasRoadEffect)
    {
        glGenVertexArrays(1, &roadEffect->vao);
        glBindVertexArray(roadEffect->vao);
        glGenBuffers(1, &roadEffect->positionBuffer);
        glGenBuffers(1, &roadEffect->colorBuffer);

        Logger::Log("Parsed ", roadEffect->travellers.positions.size() / 2, " road travellers.");
        roadEffect->travellers.TransferPositionToOpenGl(roadEffect->positionBuffer);
        roadEffect->travellers.TransferColorToOpenGl(roadEffect->colorBuffer);
        *effectData = roadEffect;
    }

    return hasRoadEffect;
}

void RoadEffect::UnloadEffect(void* effectData)
{
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    glDeleteVertexArrays(1, &roadEffect->vao);
    glDeleteBuffers(1, &roadEffect->positionBuffer);
    glDeleteBuffers(1, &roadEffect->colorBuffer);
    delete roadEffect;
}

void RoadEffect::Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds)
{
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    auto& travellers = roadEffect->travellers.positions;
    auto& roadPositions = roadEffect->roadPositions;
    for (unsigned int i = 0; i < travellers.size() / 2; i++)
    {
        vec::vec2i nearbyRoad = GetNearbyType(roadEffect->tile, roadPositions[i], TerrainTypes::ROADS);
        if (nearbyRoad.x != -1)
        {
            float height = roadEffect->tile->heightmap[nearbyRoad.x + nearbyRoad.y * subTileSize];
            float height2 = roadEffect->tile->heightmap[roadPositions[i].x + roadPositions[i].y * subTileSize];
            vec::vec2i realLastPos = subtileId * 0.10f + nearbyRoad;
            vec::vec2i realPos = subtileId * 0.10f + roadPositions[i];

            vec::vec3 bottomPos = vec::vec3((float)realLastPos.x, (float)realLastPos.y, height + 0.5f);
            vec::vec3 topPos = vec::vec3((float)realPos.x, (float)realPos.y, height2 + 0.5f);

            travellers[i * 2] = bottomPos;
            travellers[i * 2 + 1] = topPos;

            roadPositions[i] = nearbyRoad;
        }
    }

    glBindVertexArray(roadEffect->vao);
    roadEffect->travellers.TransferPositionToOpenGl(roadEffect->positionBuffer);
}

void RoadEffect::Render(void* effectData, const vec::mat4 & projectionMatrix, const vec::mat4 & mvMatrix)
{
    glLineWidth(3.0f);
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    glUseProgram(programId);
    glBindVertexArray(roadEffect->vao);

    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(mvMatrixLocation, 1, GL_FALSE, mvMatrix);

    glDrawArrays(GL_LINES, 0, roadEffect->travellers.positions.size());
    glLineWidth(1.0f);
}
