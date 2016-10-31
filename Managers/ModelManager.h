#pragma once
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>
#include "ImageManager.h"
#include "ShaderManager.h"
#include "Data\Model.h"
#include "Utils\Logger.h"
#include "Physics.h"
#include "ModelRenderStore.h"

// TODO these should be calculated and elsewhere.
const int MODELS_PER_RENDER = 65536;
const int MODEL_TEXTURE_SIZE = 512;

struct TrackingRenderStore
{
    // Backing store of static models
    ModelRenderStore backingStore;

    std::set<unsigned int> freeIds;
    std::set<unsigned int> renderedIds;

    TrackingRenderStore(GLuint mvMatrixImageId, GLuint shadingImageId)
        : backingStore(mvMatrixImageId, shadingImageId), freeIds()
    {
    }
};

// Assists with loading in 3D models
class ModelManager
{
    ImageManager* imageManager;
    long frameId;

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

    GLuint directModelRenderProgram;
    GLuint directTextureLocation;
    GLuint directShadingColorLocation;
    GLuint directSelectionFactorLocation;
    GLuint directMvLocation;
    GLuint directProjLocation;

    // Model data
    unsigned int nextModelId;
    std::vector<TextureModel> models;

    // Stores model data in preparation to rendering for dynamic and static objects.
    std::vector<TrackingRenderStore> dynamicRenderStore;
    
    void AddNewModelToRenderStore(Model* model);
    void UpdateModelInRenderStore(Model* model);
    void ZeroIndex(unsigned int modelId, unsigned int idx);

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

    // Immediately renders the specified model. Recommended for small numbers of items to avoid updating textures for each model.
    void RenderModelImmediate(const glm::mat4& projectionMatrix, Model* model);

    // Prepares for rendering the specified model given by the ID. Model will be placed in the static or dynamic list based on the physical state of the analysis body.
    void RenderModel(const glm::mat4& projectionMatrix, Model* model);

    // Finalizes rendering (and actually renders) all models.
    void FinalizeRender(const glm::mat4& projectionMatrix);

    // Initializes the OpenGL resources
    bool InitializeOpenGlResources(ShaderManager& shaderManager);

    // Finalizes the list of loaded models we know of, sending the data to OpenGL.
    void FinalizeLoadedModels();

    // Deletes all initialized OpenGL resources.
    ~ModelManager();
};
