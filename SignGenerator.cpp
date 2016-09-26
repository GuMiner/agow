#include "Math\MathOps.h"
#include "SignGenerator.h"

std::vector<SignInternal> SignGenerator::signArchetypes;

SignGenerator::SignGenerator()
{
}

bool SignGenerator::LoadModels(ModelManager* modelManager)
{
    // TODO sign models go here.
    unsigned int modelId = modelManager->LoadModel("models/rocks/dodecahedron");
    if (modelId == 0)
    {
        return false;
    }

    signArchetypes.push_back(SignInternal(BasicPhysics::CShape::ROCK_DODECAHEDRON, modelId));

    return true;
}

std::map<BasicPhysics::CShape, const std::vector<vec::vec3>*> SignGenerator::GetModelPoints(ModelManager* modelManager)
{
    std::map<BasicPhysics::CShape, const std::vector<vec::vec3>*> modelPoints;
    for (const SignInternal& signInternal : signArchetypes)
    {
        modelPoints[signInternal.shape] = &(modelManager->GetModel(signInternal.modelId).vertices.positions);
    }

    return modelPoints;
}

void SignGenerator::GetRandomSignModel(unsigned int* modelId, BasicPhysics::CShape* shape) const
{
    const SignInternal& signInternal = signArchetypes[MathOps::Rand(0, signArchetypes.size())];
    *modelId = signInternal.modelId;
    *shape = signInternal.shape;
}