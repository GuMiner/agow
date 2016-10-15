#include <glm\gtc\matrix_transform.hpp>
#include "Utils\Logger.h"
#include "BasicPhysics.h"
#include "ModelRenderStore.h"

// TODO these should be calculated and elsewhere.
const int MODELS_PER_RENDER = 16384;
const int MODEL_TEXTURE_SIZE = 256;

ModelRenderStore::ModelRenderStore(GLuint mvMatrixImageId, GLuint shadingColorAndSelectionImageId)
    : mvMatrixImageId(mvMatrixImageId), shadingColorAndSelectionImageId(shadingColorAndSelectionImageId)
{
}

unsigned int ModelRenderStore::GetInstanceCount()
{
    return shadingColorSelectionStore.size() / 2;
}

void ModelRenderStore::AddModelToStore(Model* model)
{
    if (GetInstanceCount() == MODELS_PER_RENDER)
    {
        // TODO handle this scenario so we cannot hit it.
        Logger::LogError("Cannot render model, too many renders of this model type attempted!");
        return;
    }

    glm::mat4 mvMatrix = glm::scale(BasicPhysics::GetBodyMatrix(model->body), model->scaleFactor);
    matrixStore.push_back(mvMatrix[0]);
    matrixStore.push_back(mvMatrix[1]);
    matrixStore.push_back(mvMatrix[2]);
    matrixStore.push_back(mvMatrix[3]);

    shadingColorSelectionStore.push_back(model->color);
    shadingColorSelectionStore.push_back(glm::vec4(model->selected ? 0.40f : 0.0f));
}

void ModelRenderStore::InsertInModelStore(unsigned int location, Model* model)
{
    if (location == matrixStore.size())
    {
        AddModelToStore(model);
    }
    else
    {
        glm::mat4 mvMatrix = glm::scale(BasicPhysics::GetBodyMatrix(model->body), model->scaleFactor);
        matrixStore[location * 4] = mvMatrix[0];
        matrixStore[location * 4 + 1] = mvMatrix[1];
        matrixStore[location * 4 + 2] = mvMatrix[2];
        matrixStore[location * 4 + 3] = mvMatrix[3];

        shadingColorSelectionStore[location * 2] = model->color;
        shadingColorSelectionStore[location * 2 + 1] = glm::vec4(model->selected ? 0.40f : 0.0f);
    }
}

void ModelRenderStore::Clear()
{
    matrixStore.clear();
    shadingColorSelectionStore.clear();
}