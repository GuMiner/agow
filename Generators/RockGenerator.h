#pragma once
#include <map>
#include <vector>
#include "Managers\ModelManager.h"
#include <glm\vec3.hpp>
#include "Physics.h"

struct RockInternal
{
    Physics::CShape shape;
    unsigned int modelId;

    RockInternal(Physics::CShape shape, unsigned int modelId)
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
    std::map<Physics::CShape, const std::vector<glm::vec3>*> GetModelPoints(ModelManager* modelManager);

    void GetRandomRockModel(unsigned int* modelId, Physics::CShape* shape) const;
};

