#include <glm/gtc/random.hpp>
#include "RockGenerator.h"

std::vector<RockInternal> RockGenerator::rockArchetypes;

RockGenerator::RockGenerator()
{
}

bool RockGenerator::LoadModels(ModelManager* modelManager)
{
    unsigned int modelId = modelManager->LoadModel("models/rocks/dodecahedron");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_DODECAHEDRON, modelId));

    modelId = modelManager->LoadModel("models/rocks/icosahedron");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_ICOSAHEDRON, modelId));

    modelId = modelManager->LoadModel("models/rocks/octahedron");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_OCTAHEDRON, modelId));

    modelId = modelManager->LoadModel("models/rocks/octotoad");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_OCTOTOAD, modelId));

    modelId = modelManager->LoadModel("models/rocks/sphere_one");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_SPHERE_ONE, modelId));

    modelId = modelManager->LoadModel("models/rocks/sphere_two");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_SPHERE_TWO, modelId));

    modelId = modelManager->LoadModel("models/rocks/sphere_three");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_SPHERE_THREE, modelId));

    modelId = modelManager->LoadModel("models/rocks/tetrahedron");
    if (modelId == 0)
    {
        return false;
    }

    rockArchetypes.push_back(RockInternal(PhysicsGenerator::CShape::ROCK_TETRAHEDRON, modelId));
    return true;
}

std::map<PhysicsGenerator::CShape, const std::vector<glm::vec3>*> RockGenerator::GetModelPoints(ModelManager* modelManager)
{
    std::map<PhysicsGenerator::CShape, const std::vector<glm::vec3>*> modelPoints;
    for (const RockInternal& rockInternal : rockArchetypes)
    {
        modelPoints[rockInternal.shape] = &(modelManager->GetModel(rockInternal.modelId).vertices.positions);
    }

    return modelPoints;
}

void RockGenerator::GetRandomRockModel(unsigned int* modelId, PhysicsGenerator::CShape* shape) const
{
    const RockInternal& rockInternal = rockArchetypes[glm::linearRand(0, (int)rockArchetypes.size() - 1)];
    *modelId = rockInternal.modelId;
    *shape = rockInternal.shape;
}
