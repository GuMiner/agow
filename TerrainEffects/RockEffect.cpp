#include <glm\gtc\random.hpp>
#include "Config\PhysicsConfig.h"
#include "Generators\RockGenerator.h"
#include "Managers\TerrainManager.h"
#include "Utils\Logger.h"
#include "RockEffect.h"

RockEffect::RockEffect(ModelManager* modelManager, BasicPhysics* physics)
    : modelManager(modelManager), physics(physics)
{
}

bool RockEffect::LoadBasics(ShaderManager* shaderManager)
{
    return true;
}

bool RockEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile* tile)
{
    bool hasRockEffect = false;
    RockEffectData* rockEffect = nullptr;

    // Scan the image for rock pixels.
    int rockCounter = 1;
    const long ROCK_SUBCOUNT = 8;
    const long MOVABLE_ROCK_SUBCOUNT = 16;
    for (int i = 0; i < TerrainTile::SubtileSize; i++)
    {
        for (int j = 0; j < TerrainTile::SubtileSize; j++)
        {
            int pixelId = tile->GetPixelId(glm::ivec2(i, j));
            if (tile->type[pixelId] == TerrainTypes::ROCKS)
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
                    Model model = Model();
                    BasicPhysics::CShape shape;

                    RockGenerator rockGenerator;
                    rockGenerator.GetRandomRockModel(&model.modelId, &shape);

                    // TODO randomly generated from the rock generator
                    model.color = glm::vec4(0.60f, 0.70f, 0.60f, 1.0f);

                    // TODO configurable
                    // TODO randomly generated masses.
                    float height = tile->heightmap[pixelId];
                    glm::vec2 realPos = TerrainTile::GetRealPosition(subtileId, glm::ivec2(i, j)) + glm::vec2(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));
                    model.body = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height), 0.0f);
                    model.body->setActivationState(ISLAND_SLEEPING);

                    rockEffect->rocks.push_back(model);
                    physics->DynamicsWorld->addRigidBody(model.body);
                }

                if (rockCounter % MOVABLE_ROCK_SUBCOUNT == 0)
                {
                    // Add a movable rock layer above the substrate
                    Model model = Model();
                    BasicPhysics::CShape shape;

                    RockGenerator rockGenerator;
                    rockGenerator.GetRandomRockModel(&model.modelId, &shape);

                    // TODO randomly generated from the rock generator
                    model.color = glm::vec4(0.60f, 0.70f, 0.60f, 1.0f);

                    // TODO configurable
                    // TODO randomly generated masses.
                    float height = tile->heightmap[pixelId];
                    glm::vec2 realPos = TerrainTile::GetRealPosition(subtileId, glm::ivec2(i, j)) + glm::vec2(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));
                    model.body = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height + 2.0f), 30.0f);
                    model.body->setActivationState(ISLAND_SLEEPING);

                    rockEffect->rocks.push_back(model);
                    physics->DynamicsWorld->addRigidBody(model.body);
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
    for (const Model& model : rockEffect->rocks)
    {
        // TODO -- we should not regenerate rigid bodies for rocky areas, but they (like cities) should go in a persistent store.
        // I'm leaving that off until I start random city generation. That will likely also entail refactoring in this class...
        physics->DynamicsWorld->removeRigidBody(model.body);
        physics->DeleteBody(model.body, false);
    }

    delete rockEffect;
}

void RockEffect::Simulate(const glm::ivec2 subtileId, void * effectData, float elapsedSeconds)
{
}

void RockEffect::Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    glm::mat4 projectionMatrix = perspectiveMatrix * viewMatrix;
    RockEffectData* rockEffect = (RockEffectData*)effectData;
    for (Model& model : rockEffect->rocks)
    {
        modelManager->RenderModel(projectionMatrix, &model);
    }
}
