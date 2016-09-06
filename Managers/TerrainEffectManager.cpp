#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "Config\PhysicsConfig.h"
#include "TerrainManager.h"
#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"
#include "RockGenerator.h"

TerrainEffectManager::TerrainEffectManager(ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* basicPhysics, int subTileSize)
    : shaderManager(shaderManager), modelManager(modelManager), physics(basicPhysics), subTileSize(subTileSize)
{
}

bool TerrainEffectManager::LoadBasics()
{
    if (!shaderManager->CreateShaderProgram("grassRender", &grassProgram.programId))
    {
        Logger::LogError("Failed to load the basic grass rendering shader; cannot continue.");
        return false;
    }

    grassProgram.projMatrixLocation = glGetUniformLocation(grassProgram.programId, "projMatrix");
    grassProgram.mvMatrixLocation = glGetUniformLocation(grassProgram.programId, "mvMatrix");

    if (!shaderManager->CreateShaderProgram("roadRender", &roadProgram.programId))
    {
        Logger::LogError("Failed to load the road rendering shader; cannot continue.");
        return false;
    }

    roadProgram.projMatrixLocation = glGetUniformLocation(roadProgram.programId, "projMatrix");
    roadProgram.mvMatrixLocation = glGetUniformLocation(roadProgram.programId, "mvMatrix");

    return true;
}

bool TerrainEffectManager::GetNearbyCheckOne(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
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

bool TerrainEffectManager::GetNearbyCheckTwo(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
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

bool TerrainEffectManager::GetNearbyCheckThree(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
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

bool TerrainEffectManager::GetNearbyCheckFour(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const
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

vec::vec2i TerrainEffectManager::GetNearbyType(SubTile* tile, const vec::vec2i pos, const int terrainType) const
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

void TerrainEffectManager::LoadRoadEffect(vec::vec2i pos, EffectData* effect, SubTile* tile)
{
    effect->hasRoadEffect = false;
    effect->roadEffect.tile = tile;

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
                        effect->hasRoadEffect = true;
                        float height = tile->heightmap[i + j * subTileSize];
                        float height2 = tile->heightmap[nearbyRoad.x + nearbyRoad.y * subTileSize];
                        vec::vec2i realLastPos = pos * 0.10f + nearbyRoad;
                        vec::vec2i realPos = pos * 0.10f + vec::vec2i(i, j);

                        // TODO configurable
                        vec::vec3 bottomColor = vec::vec3(0.0f, 0.50f, MathOps::Rand() * 0.20f + 0.80f);
                        vec::vec3 topColor = vec::vec3(0.40f + 0.60f * MathOps::Rand(), 0.0f, 0.20f + MathOps::Rand() * 0.40f);
                        vec::vec3 bottomPos = vec::vec3((float)realPos.x, (float)realPos.y, height + 0.5f);
                        vec::vec3 topPos = vec::vec3((float)realLastPos.x, (float)realLastPos.y, height2 + 0.5f);

                        // Add grass
                        effect->roadEffect.travellers.positions.push_back(bottomPos);
                        effect->roadEffect.travellers.positions.push_back(topPos);
                        effect->roadEffect.travellers.colors.push_back(bottomColor);
                        effect->roadEffect.travellers.colors.push_back(topColor);
                        effect->roadEffect.roadPositions.push_back(realPos);
                    }
                }
            }
        }
    }

    if (effect->hasRoadEffect)
    {
        glGenVertexArrays(1, &effect->roadEffect.vao);
        glBindVertexArray(effect->roadEffect.vao);
        glGenBuffers(1, &effect->roadEffect.positionBuffer);
        glGenBuffers(1, &effect->roadEffect.colorBuffer);

        Logger::Log("Parsed ", effect->roadEffect.travellers.positions.size() / 2, " road travellers.");
        effect->roadEffect.travellers.TransferPositionToOpenGl(effect->roadEffect.positionBuffer);
        effect->roadEffect.travellers.TransferColorToOpenGl(effect->roadEffect.colorBuffer);
    }
}

void TerrainEffectManager::LoadGrassEffect(vec::vec2i pos, EffectData* effect, SubTile* tile)
{
    effect->hasGrassEffect = false;

    float frequency = 2;

    // Scan the image for grass pixels.
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::GRASSLAND)
            {
                effect->hasGrassEffect = true;
                float height = tile->heightmap[i + j * subTileSize];
                vec::vec2i realPos = pos * 0.10f + vec::vec2i(i, j);

                // TODO configurable
                vec::vec3 bottomColor = vec::vec3(0.0f, 0.90f + MathOps::Rand() * 0.10f, 0.0f);
                vec::vec3 topColor = vec::vec3(0.0f, 0.50f + MathOps::Rand() * 0.30f, 0.20f + MathOps::Rand() * 0.60f);
                vec::vec3 bottomPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height);
                vec::vec3 topPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height + 0.15f + 1.70f * MathOps::Rand());

                // Add grass
                vec::vec3 lowerOffset = vec::vec3(0.0f);
                vec::vec3 upperOffset = vec::vec3(std::cos(2 * Constants::PI / 2), std::sin(2 * Constants::PI / 2), 0.0f);
                effect->grassEffect.grassOffsets.push_back(lowerOffset);
                effect->grassEffect.grassOffsets.push_back(upperOffset);

                effect->grassEffect.grassStalks.positions.push_back(bottomPos + lowerOffset);
                effect->grassEffect.grassStalks.positions.push_back(topPos + upperOffset);
                effect->grassEffect.grassStalks.colors.push_back(bottomColor);
                effect->grassEffect.grassStalks.colors.push_back(topColor);
                effect->grassEffect.grassStalks.ids.push_back(effect->grassEffect.grassStalks.positions.size() - 1); // Starts at 1
                effect->grassEffect.grassStalks.ids.push_back(effect->grassEffect.grassStalks.positions.size());
            }
        }
    }

    if (effect->hasGrassEffect)
    {
        // Grass vertex data.
        glGenVertexArrays(1, &effect->grassEffect.vao);
        glBindVertexArray(effect->grassEffect.vao);
        glGenBuffers(1, &effect->grassEffect.positionBuffer);
        glGenBuffers(1, &effect->grassEffect.colorBuffer);
        
        Logger::Log("Parsed ", effect->grassEffect.grassStalks.positions.size() / 2, " grass stalks.");
        effect->grassEffect.grassStalks.TransferPositionToOpenGl(effect->grassEffect.positionBuffer);
        effect->grassEffect.grassStalks.TransferColorToOpenGl(effect->grassEffect.colorBuffer);
    }
}

void TerrainEffectManager::LoadRockEffect(vec::vec2i pos, EffectData* effect, SubTile* tile)
{
    effect->hasRockEffect = false;
    effect->rockEffect.rocks.clear();

    // Scan the image for rock pixels.
    int rockCounter = 1;
    const long ROCK_SUBCOUNT = 8;
    const long MOVABLE_ROCK_SUBCOUNT = 16;
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::ROCKS)
            {
                ++rockCounter;
                if (rockCounter % ROCK_SUBCOUNT == 0)
                {
                    effect->hasRockEffect = true;

                    // Add a non-movable rock substrate.
                    ColoredPhysicalModel coloredModel;
                    BasicPhysics::CShape shape;

                    RockGenerator rockGenerator;
                    rockGenerator.GetRandomRockModel(&coloredModel.model.modelId, &shape);

                    // TODO randomly generated from the rock generator
                    coloredModel.color = vec::vec4(0.60f, 0.70f, 0.60f, 1.0f);

                    // TODO configurable
                    // TODO randomly generated masses.
                    float height = tile->heightmap[i + j * subTileSize];
                    vec::vec2 realPos = vec::vec2((float)pos.x, (float)pos.y) * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + vec::vec2((float)i + MathOps::Rand(), (float)j + MathOps::Rand());
                    coloredModel.model.rigidBody = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height), 0.0f);

                    effect->rockEffect.rocks.push_back(coloredModel);
                    physics->DynamicsWorld->addRigidBody(coloredModel.model.rigidBody);
                }

                if (rockCounter % MOVABLE_ROCK_SUBCOUNT == 0)
                {
                    // Add a movable rock layer above the substrate
                    ColoredPhysicalModel coloredModel;
                    BasicPhysics::CShape shape;

                    RockGenerator rockGenerator;
                    rockGenerator.GetRandomRockModel(&coloredModel.model.modelId, &shape);

                    // TODO randomly generated from the rock generator
                    coloredModel.color = vec::vec4(0.60f, 0.70f, 0.60f, 1.0f);

                    // TODO configurable
                    // TODO randomly generated masses.
                    float height = tile->heightmap[i + j * subTileSize];
                    vec::vec2 realPos = vec::vec2((float)pos.x, (float)pos.y) * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + vec::vec2((float)i + MathOps::Rand(), (float)j + MathOps::Rand());
                    coloredModel.model.rigidBody = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height + 2.0f), 30.0f);

                    effect->rockEffect.rocks.push_back(coloredModel);
                    physics->DynamicsWorld->addRigidBody(coloredModel.model.rigidBody);
                }
            }
        }
    }

    if (effect->hasRockEffect)
    {
        Logger::Log("Loaded ", effect->rockEffect.rocks.size(), " randomly-generated rocks in the rock field.");
    }
}

void TerrainEffectManager::UnloadGrassEffect(vec::vec2i pos)
{
    if (effectData[pos]->hasGrassEffect)
    {
        glDeleteVertexArrays(1, &effectData[pos]->grassEffect.vao);
        glDeleteBuffers(1, &effectData[pos]->grassEffect.positionBuffer);
        glDeleteBuffers(1, &effectData[pos]->grassEffect.colorBuffer);
    }
}

void TerrainEffectManager::UnloadRoadEffect(vec::vec2i pos)
{
    if (effectData[pos]->hasRoadEffect)
    {
        glDeleteVertexArrays(1, &effectData[pos]->roadEffect.vao);
        glDeleteBuffers(1, &effectData[pos]->roadEffect.positionBuffer);
        glDeleteBuffers(1, &effectData[pos]->roadEffect.colorBuffer);
    }
}

void TerrainEffectManager::UnloadRockEffect(vec::vec2i pos)
{
    if (effectData[pos]->hasRockEffect)
    {
        for (const ColoredPhysicalModel& model : effectData[pos]->rockEffect.rocks)
        {
            // TODO -- we should not regenerate rigid bodies for rocky areas, but they (like cities) should go in a persistent store.
            // I'm leaving that off until I start random city generation. That will likely also entail refactoring in this class...
            physics->DynamicsWorld->removeRigidBody(model.model.rigidBody);
        }
    }
}

bool TerrainEffectManager::LoadSubTileEffects(vec::vec2i pos, SubTile* tile)
{
    if (effectData.find(pos) != effectData.end())
    {
        // Already in cache.
        return true;
    }

    EffectData* effect = new EffectData();
    LoadGrassEffect(pos, effect, tile);
    LoadRoadEffect(pos, effect, tile);
    LoadRockEffect(pos, effect, tile);

    effectData[pos] = effect;
    return true;
}

void TerrainEffectManager::Simulate(const vec::vec2i pos, float elapsedSeconds)
{
    if (effectData.find(pos) == effectData.end())
    {
        Logger::LogWarn("Attempted to simulate terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return;
    }

    if (effectData[pos]->hasGrassEffect)
    {
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

    if (effectData[pos]->hasRoadEffect)
    {
        auto& travellers = effectData[pos]->roadEffect.travellers.positions;
        auto& roadPositions = effectData[pos]->roadEffect.roadPositions;
        for (unsigned int i = 0; i < travellers.size() / 2; i++)
        {
            vec::vec2i nearbyRoad = GetNearbyType(effectData[pos]->roadEffect.tile, roadPositions[i], TerrainTypes::ROADS);
            if (nearbyRoad.x != -1)
            {
                float height = effectData[pos]->roadEffect.tile->heightmap[nearbyRoad.x + nearbyRoad.y * subTileSize];
                float height2 = effectData[pos]->roadEffect.tile->heightmap[roadPositions[i].x + roadPositions[i].y * subTileSize];
                vec::vec2i realLastPos = pos * 0.10f + nearbyRoad;
                vec::vec2i realPos = pos * 0.10f + roadPositions[i];

                vec::vec3 bottomPos = vec::vec3((float)realLastPos.x, (float)realLastPos.y, height + 0.5f);
                vec::vec3 topPos = vec::vec3((float)realPos.x, (float)realPos.y, height2 + 0.5f);

                travellers[i * 2] = bottomPos;
                travellers[i * 2 + 1] = topPos;
            
                roadPositions[i] = nearbyRoad;
            }
        }

        glBindVertexArray(effectData[pos]->roadEffect.vao);
        effectData[pos]->roadEffect.travellers.TransferPositionToOpenGl(effectData[pos]->roadEffect.positionBuffer);
    }
}

void TerrainEffectManager::RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (effectData.find(pos) == effectData.end())
    {
        Logger::LogWarn("Attempted to render terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return; 
    }

    // Render grass
    if (effectData[pos]->hasGrassEffect)
    {
        glUseProgram(grassProgram.programId);
        glBindVertexArray(effectData[pos]->grassEffect.vao);

        glUniformMatrix4fv(grassProgram.projMatrixLocation, 1, GL_FALSE, projectionMatrix);
        glUniformMatrix4fv(grassProgram.mvMatrixLocation, 1, GL_FALSE, mvMatrix);

        glDrawArrays(GL_LINES, 0, effectData[pos]->grassEffect.grassStalks.positions.size());
    }

    // Render travellers
    if (effectData[pos]->hasRoadEffect)
    {
        glUseProgram(roadProgram.programId);
        glBindVertexArray(effectData[pos]->roadEffect.vao);

        glUniformMatrix4fv(roadProgram.projMatrixLocation, 1, GL_FALSE, projectionMatrix);
        glUniformMatrix4fv(roadProgram.mvMatrixLocation, 1, GL_FALSE, mvMatrix);

        glDrawArrays(GL_LINES, 0, effectData[pos]->roadEffect.travellers.positions.size());
    }
    
    // Render rocks in rocky areas
    if (effectData[pos]->hasRockEffect)
    {
        for (const ColoredPhysicalModel& model : effectData[pos]->rockEffect.rocks)
        {
            vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(model.model.rigidBody);
            modelManager->RenderModel(projectionMatrix, model.model.modelId, mvMatrix, model.color, false);
        }
    }
}

void TerrainEffectManager::CleanupSubTileEffects(vec::vec2i pos, bool log)
{
    // Cleanup grass rendering data.
    UnloadGrassEffect(pos);
    UnloadRoadEffect(pos);
    UnloadRockEffect(pos);
    delete effectData[pos];

    if (log)
    {
        Logger::Log("Unloaded effects ", pos.x, ", ", pos.y, ".");
    }
}

void TerrainEffectManager::UnloadSubTileEffects(vec::vec2i pos)
{
    CleanupSubTileEffects(pos, true);
    effectData.erase(pos);
}

TerrainEffectManager::~TerrainEffectManager()
{
    // Cleanup any allocated effects 
    for (auto iter = effectData.begin(); iter != effectData.end(); iter++)
    {
        CleanupSubTileEffects(iter->first, false);
    }
}
