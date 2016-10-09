#include <glm/gtc/random.hpp>
#include "SignGenerator.h"

std::vector<SignInternal> SignGenerator::signArchetypes;

SignGenerator::SignGenerator()
{
}

bool SignGenerator::LoadModels(ModelManager* modelManager)
{
    unsigned int modelId = modelManager->LoadModel("models/signs/stopSign");
    if (modelId == 0)
    {
        return false;
    }

    signArchetypes.push_back(SignInternal(BasicPhysics::CShape::SIGN_STOP, modelId));

    return true;
}

std::map<BasicPhysics::CShape, const std::vector<glm::vec3>*> SignGenerator::GetModelPoints(ModelManager* modelManager)
{
    std::map<BasicPhysics::CShape, const std::vector<glm::vec3>*> modelPoints;
    for (const SignInternal& signInternal : signArchetypes)
    {
        modelPoints[signInternal.shape] = &(modelManager->GetModel(signInternal.modelId).vertices.positions);
    }

    return modelPoints;
}

void SignGenerator::GetRandomSignModel(unsigned int* modelId, BasicPhysics::CShape* shape) const
{
    const SignInternal& signInternal = signArchetypes[glm::linearRand(0, (int)signArchetypes.size() - 1)];
    *modelId = signInternal.modelId;
    *shape = signInternal.shape;
}
