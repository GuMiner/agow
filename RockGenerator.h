#pragma once
#include <map>
#include <vector>
#include "Managers\ModelManager.h"
#include <glm\vec3.hpp>
#include "BasicPhysics.h"

struct RockInternal
{
    BasicPhysics::CShape shape;
    unsigned int modelId;

    RockInternal(BasicPhysics::CShape shape, unsigned int modelId)
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
    std::map<BasicPhysics::CShape, const std::vector<glm::vec3>*> GetModelPoints(ModelManager* modelManager);

    void GetRandomRockModel(unsigned int* modelId, BasicPhysics::CShape* shape) const;
};

