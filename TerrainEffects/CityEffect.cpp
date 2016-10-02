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

    // V0: Placed one at the start of every city area.
    // Later verisons definitely need to check for other buildings being generated and prevent overlap with other effects.
    bool hasCityEffect = false;
    CityEffectData* cityEffect = nullptr;

    // Scan the image for city pixels.
    for (int i = 0; i < subTileSize; i++)
    {
        for (int j = 0; j < subTileSize; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::CITY)
            {
                if (!hasCityEffect)
                {
                    hasCityEffect = true;
                    cityEffect = new CityEffectData();
                }

                // Get a building.
                float separationRadius;
                std::vector<ScaledPhysicalModel> building = buildingGenerator.GetRandomLowDensityBuilding(&separationRadius);

                // Move the building to be where the city part is.
                // TODO also randomly color the building.

                // TODO configurable
                float height = tile->heightmap[i + j * subTileSize];// -0.50f; // Ground inset.
                vec::vec2 realPos = vec::vec2((float)subtileId.x, (float)subtileId.y) * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + vec::vec2((float)i + MathOps::Rand(), (float)j + MathOps::Rand());

                for (unsigned int i = 0; i < building.size(); i++)
                {
                    building[i].rigidBody->translate(btVector3((float)realPos.x, (float)realPos.y, height));
                    physics->DynamicsWorld->addRigidBody(building[i].rigidBody);
                }

                cityEffect->buildings.push_back(building);

                break;
            }
        }

        if (hasCityEffect)
        {
            break;
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
        for (unsigned int j = 0; j < cityEffect->buildings[i].size(); j++)
        {
            physics->DynamicsWorld->removeRigidBody(cityEffect->buildings[i][j].rigidBody);
            physics->DeleteBody(cityEffect->buildings[i][j].rigidBody, true);
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
    for (const std::vector<ScaledPhysicalModel>& building : cityEffect->buildings)
    {
        for (const ScaledPhysicalModel& model : building)
        {
            vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(model.rigidBody) * MatrixOps::Scale(model.scaleFactor);
            modelManager->RenderModel(perspectiveMatrix * viewMatrix, model.modelId, mvMatrix, false);
        }
    }
}
