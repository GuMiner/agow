#include "Config\PhysicsConfig.h"
#include "Generators\SignGenerator.h"
#include "Managers\TerrainManager.h"
#include "Utils\Logger.h"
#include "SignEffect.h"

SignEffect::SignEffect(ModelManager* modelManager, BasicPhysics* physics, int subTileSize)
    : modelManager(modelManager), physics(physics), subTileSize(subTileSize)
{
}

bool SignEffect::LoadBasics(ShaderManager* shaderManager)
{
    // No special shaders.
    return true;
}

bool SignEffect::LoadEffect(glm::ivec2 subtileId, void** effectData, SubTile * tile)
{
    bool hasSignEffect = false;
    SignEffectData* signEfect = nullptr;

    // Scan the image for the sign setting.
    // A sign is a grass-dirt corner. TODO figure out real size.
    // TODO handle signs stradling subtiles.****
    for (int i = 1; i < subTileSize - 4; i++)
    {
        for (int j = 1; j < subTileSize - 4; j++)
        {
            if (tile->type[i + j * subTileSize] == TerrainTypes::DIRTLAND)
            {
                // See if the upper-left is grass in x and y.
                if (tile->type[(i - 1) + j * subTileSize] == TerrainTypes::GRASSLAND && tile->type[i + (j - 1) * subTileSize] == TerrainTypes::GRASSLAND)
                {
                    // See if dirtland is in the next two followed by grassland.
                    if (tile->type[(i + 1) + j * subTileSize] == TerrainTypes::DIRTLAND && tile->type[i + (j + 1) * subTileSize] == TerrainTypes::DIRTLAND)
                    {
                        if (tile->type[(i + 2) + j * subTileSize] == TerrainTypes::DIRTLAND && tile->type[i + (j + 2) * subTileSize] == TerrainTypes::DIRTLAND)
                        {
                            // Valid sign! 
                            if (!hasSignEffect)
                            {
                                hasSignEffect = true;
                                signEfect = new SignEffectData();
                            }

                            // Add a barely-movable sign shape.
                            ColoredPhysicalModel coloredModel;
                            BasicPhysics::CShape shape;

                            SignGenerator signGenerator;
                            signGenerator.GetRandomSignModel(&coloredModel.model.modelId, &shape);

                            // TODO configurable.
                            coloredModel.color = glm::vec4(0.60f, 0.70f, 0.60f, 1.0f);

                            // TODO configurable masses.
                            float height = tile->heightmap[i + j * subTileSize];
                            glm::vec2 realPos = glm::vec2((float)subtileId.x, (float)subtileId.y) * (float)(PhysicsConfig::TerrainSize / TerrainManager::Subdivisions) + glm::vec2((float)(i + 1), (float)(j + 1));
                            coloredModel.model.rigidBody = physics->GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height), 0.0f);

                            signEfect->signs.push_back(coloredModel);
                            physics->DynamicsWorld->addRigidBody(coloredModel.model.rigidBody);
                        }
                    }
                }
            }
        }
    }

    if (hasSignEffect)
    {
        Logger::Log("Loaded ", signEfect->signs.size(), " signs in the subtile.");
        *effectData = signEfect;
    }

    return hasSignEffect;
}

void SignEffect::UnloadEffect(void * effectData)
{
    SignEffectData* rockEffect = (SignEffectData*)effectData;
    for (const ColoredPhysicalModel& model : rockEffect->signs)
    {
        // TODO -- we should not regenerate signs, they should go in a persistent store.
        physics->DynamicsWorld->removeRigidBody(model.model.rigidBody);
    }

    delete rockEffect;
}

void SignEffect::Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds)
{
    // No custom simulation.
}

void SignEffect::Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    SignEffectData* rockEffect = (SignEffectData*)effectData;
    for (const ColoredPhysicalModel& model : rockEffect->signs)
    {
        glm::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(model.model.rigidBody);
        modelManager->RenderModel(perspectiveMatrix * viewMatrix, model.model.modelId, mvMatrix, model.color, false);
    }
}
