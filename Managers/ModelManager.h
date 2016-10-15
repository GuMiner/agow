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
#include "BasicPhysics.h"
#include "ModelRenderStore.h"

// TODO these should be calculated and elsewhere.
const int MODELS_PER_RENDER = 16384;
const int MODEL_TEXTURE_SIZE = 256;

struct StaticRenderStore
{
    // Backing store of static models
    ModelRenderStore backingStore;

    // The set of segments to render (in order to skip segments).
    std::list<glm::ivec2> drawSegments;

    // Items that were added this frame that need to be send to the GPU.
    std::vector<unsigned int> newItemsAdded;

    // Items drawn this frame. NOTE: Per C++ standard, this is guaranteed to be smallest to largest.
    std::set<unsigned int> drawnItems;

    StaticRenderStore(GLuint mvMatrixImageId, GLuint shadingImageId)
        : backingStore(mvMatrixImageId, shadingImageId), drawSegments(), newItemsAdded(), drawnItems()
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
    GLuint instanceOffsetLocation;
    GLuint textureLocation;
    GLuint shadingColorLocation;
    GLuint mvLocation;
    GLuint projLocation;

    // Model data
    unsigned int nextModelId;
    std::vector<TextureModel> models;

    // Stores model data in preparation to rendering for dynamic and static objects.
    std::vector<ModelRenderStore> dynamicRenderStore;
    std::vector<StaticRenderStore> staticRenderStore;
    
    void AddNewStaticModel(Model* model);
    void RenderStaticModel(Model* model);

    void FinalizeDynamicRender();
    void FinalizeStaticRender();

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
