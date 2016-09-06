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
                    if (!hasRoadEffect)
                    {
                        hasRoadEffect = true;
                        roadEffect = new RoadEffectData(tile);
                    }

                    float height = tile->heightmap[i + j * subTileSize];
                    vec::vec2i realPos = subtileId * 0.10f + vec::vec2i(i, j);

                    // TODO configurable
                    vec::vec3 bottomColor = vec::vec3(0.0f, 0.50f, MathOps::Rand() * 0.20f + 0.80f);
                    vec::vec3 topColor = vec::vec3(0.40f + 0.60f * MathOps::Rand(), 0.0f, 0.20f + MathOps::Rand() * 0.40f);
                    vec::vec3 position = vec::vec3((float)realPos.x, (float)realPos.y, height + 0.5f);
                    vec::vec2 velocity = vec::vec2(MathOps::Rand(1.0f), MathOps::Rand(1.0f)) * 40.0f;

                    vec::vec3 endPosition = position + vec::normalize(vec::vec3(velocity.x, velocity.y, 0.0f));

                    // Add road travelers
                    roadEffect->travellers.positions.push_back(position);
                    roadEffect->travellers.positions.push_back(endPosition);
                    roadEffect->travellers.colors.push_back(bottomColor);
                    roadEffect->travellers.colors.push_back(topColor);

                    roadEffect->positions.push_back(vec::vec2(position.x, position.y));
                    roadEffect->velocities.push_back(velocity);
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

float RoadEffect::MoveTraveller(const vec::vec2i subtileId, RoadEffectData* roadEffect, int i, float elapsedSeconds)
{
    roadEffect->positions[i] += (roadEffect->velocities[i] * elapsedSeconds);

    // This logic causes a slight pause when a boundary is hit, which looks logical.
    bool hitEdgeBoundary = false;
    vec::vec2i subTilePos = vec::vec2i(roadEffect->positions[i].x, roadEffect->positions[i].y) - subtileId * 0.10f;
    if (subTilePos.x < 0 || subTilePos.x >= subTileSize)
    {
        roadEffect->positions[i] -= (roadEffect->velocities[i] * elapsedSeconds);
        roadEffect->velocities[i].x *= -1.0f;
        hitEdgeBoundary = true;
    }
    
    if (subTilePos.y < 0 || subTilePos.y >= subTileSize)
    {
        roadEffect->positions[i] -= (roadEffect->velocities[i] * elapsedSeconds);
        roadEffect->velocities[i].y *= -1.0f;
        hitEdgeBoundary = true;
    }

    subTilePos = vec::vec2i(roadEffect->positions[i].x, roadEffect->positions[i].y) - subtileId * 0.10f;
    subTilePos.x = std::max(std::min(subTilePos.x, subTileSize - 1), 0);
    subTilePos.y = std::max(std::min(subTilePos.y, subTileSize - 1), 0);

    if (!hitEdgeBoundary)
    {
        // See if we went off a road. If so, correct.
        if (roadEffect->tile->type[subTilePos.x + subTilePos.y * subTileSize] != TerrainTypes::ROADS)
        {
            vec::vec2i offRoad = subTilePos;
            roadEffect->positions[i] -= (roadEffect->velocities[i] * elapsedSeconds);

            // Whichever coordinates were different, we bounce. This isn't strictly correct, as if we went through a corner of a pixel, we shouldn't bounce one axis.
            // However, the subsequent step (angular distortion) is very incorrect (or correct, depending on your viewpoint)
            bool angleXDistortion = false;
            subTilePos = vec::vec2i(roadEffect->positions[i].x, roadEffect->positions[i].y) - subtileId * 0.10f;
            if (subTilePos.x != offRoad.x)
            {
                roadEffect->velocities[i].x *= -1.0f;
                angleXDistortion = true;
            }

            bool angleYDistortion = false;
            if (subTilePos.y != offRoad.y)
            {
                roadEffect->velocities[i].y *= -1.0f;
                angleYDistortion = true;
            }

            // TODO configurable
            // Distort axis (to promote moving *away* and prevent collisions) but keep the speed the same.
            // Do nothing if both distortions are requested.
            float factor = 1.5f;
            if (angleXDistortion)
            {
                float length = vec::length(roadEffect->velocities[i]);
                roadEffect->velocities[i].y *= factor;
                roadEffect->velocities[i] = vec::normalize(roadEffect->velocities[i]) * length;
            }
            else if (angleYDistortion)
            {
                float length = vec::length(roadEffect->velocities[i]);
                roadEffect->velocities[i].x *= factor;
                roadEffect->velocities[i] = vec::normalize(roadEffect->velocities[i]) * length;
            }
        }
    }

    
    return roadEffect->tile->heightmap[subTilePos.x + subTilePos.y * subTileSize];
}

void RoadEffect::Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds)
{
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    auto& travellers = roadEffect->travellers.positions;
    for (unsigned int i = 0; i < travellers.size() / 2; i++)
    {
        float height = MoveTraveller(subtileId, roadEffect, i, elapsedSeconds);

        vec::vec3 position = vec::vec3(roadEffect->positions[i].x, roadEffect->positions[i].y, height + 0.5f);
        vec::vec3 endPosition = position + vec::normalize(vec::vec3(roadEffect->velocities[i].x, roadEffect->velocities[i].y, 0.0f));

        travellers[i * 2] = position;
        travellers[i * 2 + 1] = endPosition;
    }

    glBindVertexArray(roadEffect->vao);
    roadEffect->travellers.TransferPositionToOpenGl(roadEffect->positionBuffer);
}

void RoadEffect::Render(void* effectData, const vec::mat4 & projectionMatrix, const vec::mat4 & mvMatrix)
{
    // TODO configurable
    glLineWidth(3.0f);
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    glUseProgram(programId);
    glBindVertexArray(roadEffect->vao);

    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(mvMatrixLocation, 1, GL_FALSE, mvMatrix);

    glDrawArrays(GL_LINES, 0, roadEffect->travellers.positions.size());
    glLineWidth(1.0f);
}
