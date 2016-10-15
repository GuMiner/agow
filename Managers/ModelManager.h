#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "ImageManager.h"
#include "ShaderManager.h"
#include "Data\Model.h"
#include "Utils\Logger.h"
#include "BasicPhysics.h"

// TODO these should be calculated and elsewhere.
const int MODELS_PER_RENDER = 16384;
const int MODEL_TEXTURE_SIZE = 256;

struct ModelRenderStore
{
    // These are created from the image manager and do not need to be freed.
    GLuint mvMatrixImageId;
    GLuint shadingColorAndSelectionImageId;

    // Stores MV matrices, split into 4 vectors per matrix.
    std::vector<glm::vec4> matrixStore;

    // Stores the shading color (even indices) and selection bool ('R', odd incies)
    std::vector<glm::vec4> shadingColorSelectionStore;

    ModelRenderStore(GLuint mvMatrixImageId, GLuint shadingColorAndSelectionImageId)
        : mvMatrixImageId(mvMatrixImageId), shadingColorAndSelectionImageId(shadingColorAndSelectionImageId)
    {
    }

    unsigned int GetInstanceCount()
    {
        return shadingColorSelectionStore.size() / 2;
    }

    void AddModelToStore(Model* model)
    {
        if (GetInstanceCount() == MODELS_PER_RENDER)
        {
            // TODO handle this scenario so we cannot hit it.
            Logger::LogError("Cannot render model, too many renders of this model type attempted!");
        }

        glm::mat4 mvMatrix = glm::scale(BasicPhysics::GetBodyMatrix(model->body), model->scaleFactor);
        matrixStore.push_back(mvMatrix[0]);
        matrixStore.push_back(mvMatrix[1]);
        matrixStore.push_back(mvMatrix[2]);
        matrixStore.push_back(mvMatrix[3]);

        shadingColorSelectionStore.push_back(model->color);
        shadingColorSelectionStore.push_back(glm::vec4(model->selected ? 0.40f : 0.0f));
    }

    void Clear()
    {
        matrixStore.clear();
        shadingColorSelectionStore.clear();
    }
};

// Assists with loading in 3D models
class ModelManager
{
    ImageManager* imageManager;

    // Rendering data
    GLuint vao;
    GLuint uvBuffer;
    GLuint positionBuffer;
    GLuint indexBuffer;

    GLuint modelRenderProgram;

    // In-shader locations
    GLuint textureLocation;
    GLuint shadingColorLocation;
    GLuint mvLocation;
    GLuint projLocation;

    // Model data
    unsigned int nextModelId;
    std::vector<TextureModel> models;

    // Stores model data in preparation to rendering for dynamic objects.
    std::vector<ModelRenderStore> dynamicRenderStore;

public:
    // Clears the next model ID and initializes the local reference to the image manager.
    ModelManager(ImageManager* imageManager);

    // Loads a new textured OBJ model, returning the model ID. Returns 0 on failure.
    unsigned int LoadModel(const char* rootFilename);

    // Retrieves a 3D model, returning the model ID.
    const TextureModel& GetModel(unsigned int id);

    // Retrieves the model ID given the name used to load the model.
    unsigned int GetModelId(std::string name) const;

    unsigned int GetCurrentModelCount() const;

    // Prepares for rendering the specified model given by the ID. Model will be placed in the static or dynamic list based on the physical state of the analysis body.
    void RenderModel(Model* model);

    // Forces rendering of the model in the dynamic list, which will apply any modifications done to the model.
    void RenderDynamicModel(Model* model);

    // Finalizes rendering (and actually renders) all models.
    void FinalizeRender(const glm::mat4& projectionMatrix);

    // Initializes the OpenGL resources
    bool InitializeOpenGlResources(ShaderManager& shaderManager);

    // Finalizes the list of loaded models we know of, sending the data to OpenGL.
    void FinalizeLoadedModels();

    // Deletes all initialized OpenGL resources.
    ~ModelManager();
};
