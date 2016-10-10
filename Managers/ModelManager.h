#pragma once
#include <map>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>
#include "ImageManager.h"
#include "ShaderManager.h"
#include "Data\Model.h"

struct PosUvPair
{
    unsigned int positionId;
    unsigned int uvId;
};

struct ModelRenderStore
{
    // Stores MV matrices, split into 4 vectors per matrix.
    std::vector<glm::vec4> matrixStore;

    // Stores the shading color (even indices) and selection bool ('R', odd incies)
    std::vector<glm::vec4> shadingColorSelectionStore;

    unsigned int GetInstanceCount()
    {
        return shadingColorSelectionStore.size() / 2;
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

    // Buffers. These are created from the image manager, and do not need to be freed.
    GLuint mvMatrixImageId;
    GLuint shadingColorAndSelectionImageId;

    // Model data
    unsigned int nextModelId;
    std::vector<TextureModel> models;

    // Stores model data in preparation to rendering.
    std::vector<ModelRenderStore> modelRenderStore;

    // Temporary loading structures.
    std::vector<glm::vec2> rawUvs;
    std::vector<PosUvPair> rawIndices;
    std::map<unsigned int, unsigned int> indexUvMap; // [positionId] = UV Id. Maps rawIndices for faster loading access.
    std::map<unsigned int, std::vector<PosUvPair>> uvVertexRemapping;

    // Given a position index and UV coordinate index, returns the actual index to use.
    unsigned int GetActualVertexIndex(unsigned int positionIdx, unsigned int uvIdx, universalVertices& vertices);

    // Parses an individual line of an OBJ model file.
    bool ParseLine(const std::vector<std::string>& line, universalVertices& vertices);

    // Loads an OBJ model into the specified vertices, returning true on success.
    // Note that the OBJ model must fully specify all positions / UVs *before* any indices.
    bool LoadModel(const char* objFilename, universalVertices& vertices, int* rawPointCount, glm::vec3* minBounds, glm::vec3* maxBounds);

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

    // Prepares for rendering the specified model given by the ID.
    void RenderModel(const glm::mat4& projectionMatrix, unsigned int id, glm::mat4& mvMatrix, bool selected);

    // Prepares for rendering the specified model given by the ID, using the given color.
    void RenderModel(const glm::mat4& projectionMatrix, unsigned int id, glm::mat4& mvMatrix, glm::vec4 shadingColor, bool selected);

    // Finalizes rendering (and actually renders) all models.
    void FinalizeRender(const glm::mat4& projectionMatrix);

    // Initializes the OpenGL resources
    bool InitializeOpenGlResources(ShaderManager& shaderManager);

    // Finalizes the list of loaded models we know of, sending the data to OpenGL.
    void FinalizeLoadedModels();

    // Deletes all initialized OpenGL resources.
    ~ModelManager();
};
