#pragma once
#include <map>
#include <vector>
#include <glm\vec3.hpp>
#include "Managers\ModelManager.h"
#include "BasicPhysics.h"

struct SignInternal
{
    BasicPhysics::CShape shape;
    unsigned int modelId;

    SignInternal(BasicPhysics::CShape shape, unsigned int modelId)
        : shape(shape), modelId(modelId)
    {
    }
};

class SignGenerator
{
    static std::vector<SignInternal> signArchetypes;

public:
    SignGenerator();
    static bool LoadModels(ModelManager* modelManager);

    // Returns the model points so that physics can properly work on these models.
    std::map<BasicPhysics::CShape, const std::vector<glm::vec3>*> GetModelPoints(ModelManager* modelManager);

    void GetRandomSignModel(unsigned int* modelId, BasicPhysics::CShape* shape) const;
};

