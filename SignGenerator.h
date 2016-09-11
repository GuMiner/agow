#pragma once
#include <map>
#include <vector>
#include "Managers\ModelManager.h"
#include "Math\Vec.h"
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
    std::map<BasicPhysics::CShape, const std::vector<vec::vec3>*> GetModelPoints(ModelManager* modelManager);

    void GetRandomSignModel(unsigned int* modelId, BasicPhysics::CShape* shape) const;
};

