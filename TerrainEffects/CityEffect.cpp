#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\random.hpp>
#include "Config\PhysicsConfig.h"
#include "Generators\BuildingGenerator.h"
#include "Generators\ColorGenerator.h"
#include "Generators\PhysicsGenerator.h"
#include "Managers\TerrainManager.h"
#include "Math\PhysicsOps.h"
#include "Utils\Logger.h"
#include "CityEffect.h"

CityStats CityEffect::stats = CityStats();

CityEffect::CityEffect(ModelManager* modelManager, Physics* physics, const std::string& cacheFolder)
    : modelManager(modelManager), physics(physics) // TODO use the cache to avoid regenerating buildings.
{
}

bool CityEffect::LoadBasics(ShaderManager* shaderManager)
{
    // TODO
    return true;
}

bool CityEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile)
{
    BuildingGenerator buildingGenerator(modelManager, physics);

    // V1: Placed in square regions according to the size of the building generated.
    std::vector<std::tuple<int, int, int>> squareRegionsFound;

    bool* checkedTiles = new bool[TerrainTile::SubtileSize * TerrainTile::SubtileSize];
    for (int i = 0; i < TerrainTile::SubtileSize * TerrainTile::SubtileSize; i++)
    {
        checkedTiles[i] = false;
    }

    // TODO configurable
    int minRegionSize = 11;

    // Scan the image for city pixels, forming valid square regions to put buildings within.
    for (int i = 0; i < TerrainTile::SubtileSize; i++)
    {
        for (int j = 0; j < TerrainTile::SubtileSize; j++)
        {
            int pixelId = tile->GetPixelId(glm::ivec2(i, j));
            if (tile->type[pixelId] == TerrainTypes::CITY && !checkedTiles[pixelId])
            {
                // This is a new square region. Add it to the regions found.
                checkedTiles[pixelId] = true;
                int regionSize = 1;

                bool endOfRegion = false;
                auto isInBounds = [&]() { return (i + regionSize < TerrainTile::SubtileSize && j + regionSize < TerrainTile::SubtileSize); };
                while (!endOfRegion && isInBounds())
                {
                    // Check horizontal. Also go one additional pixel to get the corner pixel.
                    for (int m = i; m < i + regionSize; m++)
                    {
                        int posId = tile->GetPixelId(glm::ivec2(m, (j + regionSize)));
                        if (tile->type[posId] != TerrainTypes::CITY || checkedTiles[posId])
                        {
                            endOfRegion = true;
                            break;
                        }
                    }

                    if (!endOfRegion)
                    {
                        // Check vertical.
                        for (int m = j; m < j + regionSize - 1; m++)
                        {
                            int posId = tile->GetPixelId(glm::ivec2((i + regionSize), m));
                            if (tile->type[posId] != TerrainTypes::CITY || checkedTiles[posId])
                            {
                                endOfRegion = true;
                                break;
                            }
                        }
                    }

                    if (!endOfRegion)
                    {
                        // We now can add both areas we just scanned onto the checked tiles list.
                        for (int m = i; m < i + regionSize; m++)
                        {
                            checkedTiles[tile->GetPixelId(glm::ivec2(m, (j + regionSize)))] = true;
                        }

                        for (int m = j; m < j + regionSize - 1; m++)
                        {
                            checkedTiles[tile->GetPixelId(glm::ivec2((i + regionSize), m))] = true;
                        }

                        // Our region size includes the existing pixel, but in our calculations above it included an additional horizontal and vertical
                        //  column. We can therefore add an additional row and continue searching for the end of the city region.
                        ++regionSize;
                    }
                }

                // Add the valid region.
                if (regionSize >= minRegionSize)
                {
                    squareRegionsFound.push_back(std::tuple<int, int, int>(i, j, regionSize));
                }
            }
        }
    }

    delete[] checkedTiles;

    if (squareRegionsFound.size() != 0)
    {
        Logger::Log("Found ", squareRegionsFound.size(), " regions to put a city building within.");
    }

    // TODO configurable
    // Add a building to all regions found with at least a size of '5' (building size), stuck in the middle.
    bool hasCityEffect = false;
    CityEffectData* cityEffect = nullptr;
    for (auto iter = squareRegionsFound.cbegin(); iter != squareRegionsFound.cend(); iter++)
    {
        if (!hasCityEffect)
        {
            hasCityEffect = true;
            cityEffect = new CityEffectData();
            cityEffect->isHighDensity = glm::linearRand(0.0f, 1.0f) > 0.75f; // TODO configurable.
        }

        int buildingXPos = std::get<0>(*iter);
        int buildingYPos = std::get<1>(*iter);
        int regionSize = std::get<2>(*iter);
        for (int m = 0; m < regionSize / 11; m++)
        {
            for (int n = 0; n < regionSize / 11; n++)
            {
                int xPos = buildingXPos + (1 + m) * 11;
                int yPos = buildingYPos + (1 + n) * 11;

                // Get a building.
                float buildingFootprintSize, buildingHeight;
                float height = tile->heightmap[buildingXPos + buildingYPos * TerrainTile::SubtileSize];// -0.50f; // Ground inset, TODO configurable.
                glm::vec2 realPos = TerrainTile::GetRealPosition(subtileId, glm::ivec2(xPos, yPos));
                glm::vec3 offset((float)realPos.x, (float)realPos.y, height);
                Building building;
                building.segments = cityEffect->isHighDensity ? 
                    buildingGenerator.GetRandomHighDensityBuilding(offset, &buildingFootprintSize, &buildingHeight) :
                    buildingGenerator.GetRandomLowDensityBuilding(offset, &buildingFootprintSize, &buildingHeight);
                building.color = ColorGenerator::GetBuildingColor();
                for (unsigned int i = 0; i < building.segments.size(); i++)
                {
                    building.segments[i].color = building.color;
                }
                
                building.separated = false;

                // Add in a detector to make the building respond to physics when necessary.
                btVector3 halfExtents = btVector3(buildingFootprintSize / 2.0f, buildingFootprintSize / 2.0f, buildingHeight / 2.0f);
                btCollisionShape* collisionShape = new btBoxShape(halfExtents);
                btRigidBody* analysisBody = PhysicsGenerator::GetGhostObject(collisionShape, PhysicsOps::Convert(offset + glm::vec3(0, 0, buildingHeight / 2.0f)));
                analysisBody->setActivationState(ISLAND_SLEEPING);

                BuildingCollisionCallbackData* collisionData = new BuildingCollisionCallbackData();
                collisionData->effect = cityEffect;
                collisionData->buildingId = cityEffect->buildings.size();

                analysisBody->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::BUILDING_COVER, this, collisionData, true));
                physics->AddBody(analysisBody);

                for (unsigned int i = 0; i < building.segments.size(); i++)
                {
                    building.segments[i].analysisBody = analysisBody;
                }

                cityEffect->buildings.push_back(building);
                if (cityEffect->buildings.size() == 50)
                {
                    // Too many buildings!
                    Logger::Log("Hit building limit at tile [", subtileId.x, " ", subtileId.y, "].");
                    break;
                }
            }
        }
    }

    if (hasCityEffect)
    {
        Logger::Log("Loaded ", cityEffect->buildings.size(), " randomly-generated buildings in the city areas.");
        *effectData = cityEffect;
    }

    return hasCityEffect;
}

void CityEffect::UnloadEffect(void* effectData)
{
    CityEffectData* cityEffect = (CityEffectData*)effectData;
    
    // Remove the physics bodies and custom collision shapes.
    for (unsigned int i = 0; i < cityEffect->buildings.size(); i++)
    {
        if (!cityEffect->buildings[i].separated)
        {
            physics->RemoveBody(cityEffect->buildings[i].segments[0].analysisBody);
            physics->DeleteBody(cityEffect->buildings[i].segments[0].analysisBody, true);
        }

        for (unsigned int j = 0; j < cityEffect->buildings[i].segments.size(); j++)
        {
            if (cityEffect->buildings[i].separated)
            {
                physics->RemoveBody(cityEffect->buildings[i].segments[j].body);
            }
            physics->DeleteBody(cityEffect->buildings[i].segments[j].body, true);
        }
    }

    delete cityEffect;
}

void CityEffect::Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds)
{
}

void CityEffect::Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    sf::Clock clock;
    glm::mat4 projectionMatrix = perspectiveMatrix * viewMatrix;
    CityEffectData* cityEffect = (CityEffectData*)effectData;
    for (Building& building : cityEffect->buildings)
    {
        stats.segmentsRendered += building.segments.size();
        for (Model& model : building.segments)
        {
            modelManager->RenderModel(projectionMatrix, &model);
        }
    }
    
    stats.usRenderTime += (long)clock.getElapsedTime().asMicroseconds();
    stats.tilesRendered++;
}

void CityEffect::LogStats()
{
    Logger::Log("City Rendering: ", stats.usRenderTime, " us, ", stats.segmentsRendered, " segments, ", stats.tilesRendered, " tiles.");
    stats.Reset();
}

void CityEffect::Callback(UserPhysics::ObjectType collidingObject, void* callbackSpecificData)
{
    BuildingCollisionCallbackData* callbackData = (BuildingCollisionCallbackData*)callbackSpecificData;
    if (callbackData->effect->buildings[callbackData->buildingId].separated)
    {
        // This building has already been separated and has been called by multiple objects at once.
        return;
    }

    // Doesn't matter what we collided with, this building is being split into segments now.
    auto& segments = callbackData->effect->buildings[callbackData->buildingId].segments;

    physics->RemoveBody(segments[0].analysisBody);
    physics->DeleteBody(segments[0].analysisBody, true);

    for (unsigned int i = 0; i < segments.size(); i++)
    {
        segments[i].analysisBody = segments[i].body;
        segments[i].body->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::BUILDING_SEGMENT));
        physics->AddBody(segments[i].body);
    }

    callbackData->effect->buildings[callbackData->buildingId].separated = true;
}
