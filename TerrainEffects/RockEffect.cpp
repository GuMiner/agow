#include "Config\PhysicsConfig.h"
#include "Managers\TerrainManager.h"
#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "RockGenerator.h"
#include "RockEffect.h"

RockEffect::RockEffect(ModelManager* modelManager, BasicPhysics* physics, int subTileSize)
    : modelManager(modelManager), physics(physics), subTileSize(subTileSize)
{
}

bool RockEffect::LoadBasics(ShaderManager* shaderManager)
{
    return true;
}

bool RockEffect::LoadEffect(vec::vec2i subtileId, void** effectData, SubTile * tile)
{
    bool hasRockEffect = false;
    RockEffectData* rockEffect = nullptr;

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
                    if (!hasRockEffect)
                    {
                        hasRockEffect = true;
                        rockEffect = new RockEffectData();
                    }

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
                    vec::vec2 realPos = vec::vec2((float)subtileId.x, (float)subtileId.y) * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + vec::vec2((float)i + MathOps::Rand(), (float)j + MathOps::Rand());
                    coloredModel.model.rigidBody = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height), 0.0f);
                    coloredModel.model.rigidBody->setActivationState(ISLAND_SLEEPING);

                    rockEffect->rocks.push_back(coloredModel);
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
                    vec::vec2 realPos = vec::vec2((float)subtileId.x, (float)subtileId.y) * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + vec::vec2((float)i + MathOps::Rand(), (float)j + MathOps::Rand());
                    coloredModel.model.rigidBody = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height + 2.0f), 30.0f);
                    coloredModel.model.rigidBody->setActivationState(ISLAND_SLEEPING);
                    rockEffect->rocks.push_back(coloredModel);
                    physics->DynamicsWorld->addRigidBody(coloredModel.model.rigidBody);
                }
            }
        }
    }

    if (hasRockEffect)
    {
        Logger::Log("Loaded ", rockEffect->rocks.size(), " randomly-generated rocks in the rock field.");
        *effectData = rockEffect;
    }

    return hasRockEffect;
}

void RockEffect::UnloadEffect(void * effectData)
{
    RockEffectData* rockEffect = (RockEffectData*)effectData;
    for (const ColoredPhysicalModel& model : rockEffect->rocks)
    {
        // TODO -- we should not regenerate rigid bodies for rocky areas, but they (like cities) should go in a persistent store.
        // I'm leaving that off until I start random city generation. That will likely also entail refactoring in this class...
        physics->DynamicsWorld->removeRigidBody(model.model.rigidBody);
        physics->DeleteBody(model.model.rigidBody, false);
    }

    delete rockEffect;
}

void RockEffect::Simulate(const vec::vec2i subtileId, void * effectData, float elapsedSeconds)
{
}

void RockEffect::Render(void* effectData, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix, const vec::mat4& modelMatrix)
{
    RockEffectData* rockEffect = (RockEffectData*)effectData;
    for (const ColoredPhysicalModel& model : rockEffect->rocks)
    {
        vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(model.model.rigidBody);
        modelManager->RenderModel(perspectiveMatrix * viewMatrix, model.model.modelId, mvMatrix, model.color, false);
    }
}
