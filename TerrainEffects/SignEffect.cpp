#include "Config\PhysicsConfig.h"
#include "Generators\SignGenerator.h"
#include "Generators\PhysicsGenerator.h"
#include "Managers\TerrainManager.h"
#include "logging\Logger.h"
#include "SignEffect.h"

SignEffect::SignEffect(ModelManager* modelManager, Physics* physics)
    : modelManager(modelManager), physics(physics)
{
}

bool SignEffect::LoadBasics(ShaderFactory* shaderManager)
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
    for (int i = 1; i < TerrainTile::SubtileSize - 4; i++)
    {
        for (int j = 1; j < TerrainTile::SubtileSize - 4; j++)
        {
            int pixelId = tile->GetPixelId(glm::ivec2(i, j));
            if (tile->type[pixelId] == TerrainTypes::DIRTLAND)
            {
                // See if the upper-left is grass in x and y.
                if (tile->type[(i - 1) + j * TerrainTile::SubtileSize] == TerrainTypes::GRASSLAND && tile->type[i + (j - 1) * TerrainTile::SubtileSize] == TerrainTypes::GRASSLAND)
                {
                    // See if dirtland is in the next two followed by grassland.
                    if (tile->type[(i + 1) + j * TerrainTile::SubtileSize] == TerrainTypes::DIRTLAND && tile->type[i + (j + 1) * TerrainTile::SubtileSize] == TerrainTypes::DIRTLAND)
                    {
                        if (tile->type[(i + 2) + j * TerrainTile::SubtileSize] == TerrainTypes::DIRTLAND && tile->type[i + (j + 2) * TerrainTile::SubtileSize] == TerrainTypes::DIRTLAND)
                        {
                            // Valid sign! 
                            if (!hasSignEffect)
                            {
                                hasSignEffect = true;
                                signEfect = new SignEffectData();
                            }

                            // Add a barely-movable sign shape.
                            Model model = Model();
                            PhysicsGenerator::CShape shape;

                            SignGenerator signGenerator;
                            signGenerator.GetRandomSignModel(&model.modelId, &shape);

                            // TODO configurable.
                            model.color = glm::vec4(0.60f, 0.70f, 0.60f, 1.0f);

                            // TODO configurable masses.
                            float height = tile->heightmap[pixelId];
                            glm::vec2 realPos = TerrainTile::GetRealPosition(subtileId, glm::ivec2(i + 1, j + 1));
                            model.body = PhysicsGenerator::GetDynamicBody(shape, btVector3(realPos.x, realPos.y, height), 0.0f);

                            signEfect->signs.push_back(model);
                            physics->AddBody(model.body);
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
    for (const Model& model : rockEffect->signs)
    {
        // TODO -- we should not regenerate signs, they should go in a persistent store.
        physics->RemoveBody(model.body);
    }

    delete rockEffect;
}

void SignEffect::Simulate(const glm::ivec2 subtileId, void* effectData, float elapsedSeconds)
{
    // No custom simulation.
}

void SignEffect::Render(void* effectData, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    glm::mat4 projectionMatrix = perspectiveMatrix * viewMatrix;
    SignEffectData* rockEffect = (SignEffectData*)effectData;
    for (Model& model : rockEffect->signs)
    {
        modelManager->RenderModel(projectionMatrix, &model);
    }
}

void SignEffect::LogStats()
{
    // Minimal impact, nothing special here.
}
