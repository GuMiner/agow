#include "Config\PhysicsConfig.h"
#include "Math\MathOps.h"
#include "Managers\TerrainManager.h"
#include "Utils\Logger.h"
#include "BuildingGenerator.h"
#include "CityEffect.h"

CityEffect::CityEffect(ModelManager* modelManager, BasicPhysics* physics, const std::string& cacheFolder, int subTileSize)
    : modelManager(modelManager), physics(physics), subTileSize(subTileSize) // TODO use the cache to avoid regeneraitng buildings.
{
}

bool CityEffect::LoadBasics(ShaderManager* shaderManager)
{
    // TODO
    return true;
}

bool CityEffect::LoadEffect(vec::vec2i subtileId, void** effectData, SubTile* tile)
{
    BuildingGenerator buildingGenerator(modelManager, physics);

    // V1: Placed in square regions according to the size of the building generated.
    std::vector<std::tuple<int, int, int>> squareRegionsFound;

    bool* checkedTiles = new bool[subTileSize * subTileSize];
    for (int i = 0; i < subTileSize * subTileSize; i++)
    {
        checkedTiles[i] = false;
    }

    // TODO configurable
    int minRegionSize = 11;

    // Scan the image for city pixels, forming valid square regions to put buildings within.
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::CITY && !checkedTiles[i + j * subTileSize])
            {
                // This is a new square region. Add it to the regions found.
                checkedTiles[i + j * subTileSize] = true;
                int regionSize = 1;

                bool endOfRegion = false;
                auto isInBounds = [&]() { return (i + regionSize < subTileSize && j + regionSize < subTileSize); };
                while (!endOfRegion && isInBounds())
                {
                    // Check horizontal. Also go one additional pixel to get the corner pixel.
                    for (int m = i; m < i + regionSize; m++)
                    {
                        int posId = m + (j + regionSize) * subTileSize;
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
                            int posId = (i + regionSize) + m * subTileSize;
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
                            checkedTiles[m + (j + regionSize) * subTileSize] = true;
                        }

                        for (int m = j; m < j + regionSize - 1; m++)
                        {
                            checkedTiles[(i + regionSize) + m * subTileSize] = true;
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
                float separationRadius;
                float height = tile->heightmap[buildingXPos + buildingYPos * subTileSize];// -0.50f; // Ground inset, TODO configurable.
                vec::vec2 realPos = vec::vec2((float)subtileId.x, (float)subtileId.y) * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + vec::vec2((float)xPos, (float)yPos);
                vec::vec3 offset((float)realPos.x, (float)realPos.y, height);
                Building building;
                building.segments = buildingGenerator.GetRandomLowDensityBuilding(offset, &separationRadius);
                building.color = vec::vec4(MathOps::Rand(), MathOps::Rand(), MathOps::Rand(), 0.50f + MathOps::Rand() / 2.0f);

                // Move the building to be where the city part is.
                // TODO also randomly color the building.
                for (unsigned int i = 0; i < building.segments.size(); i++)
                {
                    //physics->DynamicsWorld->addRigidBody(building.segments[i].rigidBody);
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
        for (unsigned int j = 0; j < cityEffect->buildings[i].segments.size(); j++)
        {
            //physics->DynamicsWorld->removeRigidBody(cityEffect->buildings[i].segments[j].rigidBody);
            physics->DeleteBody(cityEffect->buildings[i].segments[j].rigidBody, true);
        }
    }

    delete cityEffect;
}

void CityEffect::Simulate(const vec::vec2i subtileId, void* effectData, float elapsedSeconds)
{
}

void CityEffect::Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix)
{
    CityEffectData* cityEffect = (CityEffectData*)effectData;
    for (const Building& building : cityEffect->buildings)
    {
        for (const ScaledPhysicalModel& model : building.segments)
        {
            vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(model.rigidBody);
            mvMatrix = mvMatrix * MatrixOps::Scale(model.scaleFactor);
            modelManager->RenderModel(perspectiveMatrix * viewMatrix, model.modelId, mvMatrix, building.color, false);
        }
    }
}
