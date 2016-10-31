#pragma once
#include <map>
#include <vector>
#include <glm\vec3.hpp>
#include "Managers\ModelManager.h"
#include "PhysicsGenerator.h"
#include "Physics.h"

struct RockInternal
{
    PhysicsGenerator::CShape shape;
    unsigned int modelId;

    RockInternal(PhysicsGenerator::CShape shape, unsigned int modelId)
        : shape(shape), modelId(modelId)
    {
    }
};

class RockGenerator
{
    static std::vector<RockInternal> rockArchetypes;

public:
    RockGenerator();
    static bool LoadModels(ModelManager* modelManager);

    // Returns the model points so that physics can properly work on these models.
    std::map<PhysicsGenerator::CShape, const std::vector<glm::vec3>*> GetModelPoints(ModelManager* modelManager);

    void GetRandomRockModel(unsigned int* modelId, PhysicsGenerator::CShape* shape) const;
};

