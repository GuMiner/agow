#include <glm\mat4x4.hpp>
#include <glm\gtc\random.hpp>
#include <SFML\System.hpp>
#include <algorithm>
#include "Generators\ColorGenerator.h"
#include "logging\Logger.h"
#include "RoadEffect.h"

RoadStats RoadEffect::stats = RoadStats();

RoadEffect::RoadEffect()
{
}

bool RoadEffect::LoadBasics(ShaderFactory* shaderManager)
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

bool RoadEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile)
{
    bool hasRoadEffect = false;
    RoadEffectData* roadEffect = nullptr;

    // Scan the image for road pixels.
    int roadCounter = 1;
    const long ROAD_SUBCOUNT = 10;
    for (int i = 0; i < TerrainTile::SubtileSize; i++)
    {
        for (int j = 0; j <  TerrainTile::SubtileSize; j++)
        {
            int pixelId = tile->GetPixelId(glm::ivec2(i, j));
            if (tile->type[pixelId] == TerrainTypes::ROADS)
            {
                ++roadCounter;

                if (roadCounter % ROAD_SUBCOUNT == 0)
                {
                    if (!hasRoadEffect)
                    {
                        hasRoadEffect = true;
                        roadEffect = new RoadEffectData(tile);
                    }

                    float height = tile->heightmap[pixelId];
                    
                    // TODO configurable
                    glm::vec3 bottomColor = ColorGenerator::GetTravellerColor();
                    glm::vec3 topColor = ColorGenerator::GetTravellerColor() * 1.10f;
                    glm::vec3 position = glm::vec3((float)i, (float)j, height + 0.1f);
                    glm::vec2 velocity = glm::vec2(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f)) * 1.0f;

                    glm::vec3 endPosition = position + glm::normalize(glm::vec3(velocity.x, velocity.y, 0.0f));

                    // Add road travelers
                    roadEffect->travellers.positions.push_back(position);
                    roadEffect->travellers.positions.push_back(endPosition);
                    roadEffect->travellers.colors.push_back(bottomColor);
                    roadEffect->travellers.colors.push_back(topColor);

                    roadEffect->positions.push_back(glm::vec2(position.x, position.y));
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
        roadEffect->travellers.TransferStaticColorToOpenGl(roadEffect->colorBuffer);
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

float RoadEffect::MoveTraveller(const glm::ivec2 subtileId, RoadEffectData* roadEffect, int i, float elapsedSeconds)
{
    roadEffect->positions[i] += (roadEffect->velocities[i] * elapsedSeconds);

    // This logic causes a slight pause when a boundary is hit, which looks logical.
    bool hitEdgeBoundary = false;
    glm::ivec2 subTilePos = glm::ivec2(roadEffect->positions[i].x, roadEffect->positions[i].y);
    if (subTilePos.x < 0 || subTilePos.x >= TerrainTile::SubtileSize)
    {
        roadEffect->positions[i] -= (roadEffect->velocities[i] * elapsedSeconds);
        roadEffect->velocities[i].x *= -1.0f;
        hitEdgeBoundary = true;
    }
    
    if (subTilePos.y < 0 || subTilePos.y >= TerrainTile::SubtileSize)
    {
        roadEffect->positions[i] -= (roadEffect->velocities[i] * elapsedSeconds);
        roadEffect->velocities[i].y *= -1.0f;
        hitEdgeBoundary = true;
    }

    // Ensure we're within bounds.
    subTilePos = glm::ivec2(roadEffect->positions[i].x, roadEffect->positions[i].y);
    subTilePos.x = std::max(std::min(subTilePos.x, TerrainTile::SubtileSize - 1), 0);
    subTilePos.y = std::max(std::min(subTilePos.y, TerrainTile::SubtileSize - 1), 0);

    if (!hitEdgeBoundary)
    {
        // See if we went off a road. If so, correct.
        int pixelId = roadEffect->tile->GetPixelId(subTilePos);
        if (roadEffect->tile->type[pixelId] != TerrainTypes::ROADS)
        {
            glm::ivec2 offRoad = subTilePos;
            roadEffect->positions[i] -= (roadEffect->velocities[i] * elapsedSeconds);

            // Whichever coordinates were different, we bounce. This isn't strictly correct, as if we went through a corner of a pixel, we shouldn't bounce one axis.
            // However, the subsequent step (angular distortion) is very incorrect (or correct, depending on your viewpoint)
            bool angleXDistortion = false;
            subTilePos = glm::ivec2(roadEffect->positions[i].x, roadEffect->positions[i].y);
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
                float length = glm::length(roadEffect->velocities[i]);
                roadEffect->velocities[i].y *= factor;
                roadEffect->velocities[i] = glm::normalize(roadEffect->velocities[i]) * length;
            }
            else if (angleYDistortion)
            {
                float length = glm::length(roadEffect->velocities[i]);
                roadEffect->velocities[i].x *= factor;
                roadEffect->velocities[i] = glm::normalize(roadEffect->velocities[i]) * length;
            }
        }
    }

    return roadEffect->tile->heightmap[roadEffect->tile->GetPixelId(subTilePos)];
}

void RoadEffect::Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds)
{
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    auto& travellers = roadEffect->travellers.positions;
    for (unsigned int i = 0; i < travellers.size() / 2; i++)
    {
        float height = MoveTraveller(subtileId, roadEffect, i, elapsedSeconds);

        glm::vec3 position = glm::vec3(roadEffect->positions[i].x, roadEffect->positions[i].y, height + 0.f);
        glm::vec3 endPosition = position + glm::normalize(glm::vec3(roadEffect->velocities[i].x, roadEffect->velocities[i].y, 0.0f));

        travellers[i * 2] = position;
        travellers[i * 2 + 1] = endPosition;
    }

    glBindVertexArray(roadEffect->vao);
    roadEffect->travellers.TransferUpdatePositionToOpenGl(roadEffect->positionBuffer);
}

void RoadEffect::Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    sf::Clock clock;

    // TODO configurable
    glLineWidth(3.0f);
    RoadEffectData* roadEffect = (RoadEffectData*)effectData;
    glUseProgram(programId);
    glBindVertexArray(roadEffect->vao);

    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, &perspectiveMatrix[0][0]);
    glUniformMatrix4fv(mvMatrixLocation, 1, GL_FALSE, &(viewMatrix * modelMatrix)[0][0]);

    glDrawArrays(GL_LINES, 0, roadEffect->travellers.positions.size());
    glLineWidth(1.0f);

    stats.usRenderTime += clock.getElapsedTime().asMicroseconds();
    stats.travellersRendered += roadEffect->positions.size();
    stats.tilesRendered++;
}

void RoadEffect::LogStats()
{
    Logger::Log("Road Rendering: ", stats.usRenderTime, " us, ", stats.travellersRendered, " travellers, ", stats.tilesRendered, " tiles.");
    stats.Reset();
}
