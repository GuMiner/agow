#include <limits>
#include <glm\gtc\matrix_transform.hpp>
#include "Utils\Logger.h"
#include "ModelLoader.h"
#include "ModelManager.h"

ModelManager::ModelManager(ImageManager* imageManager)
    : imageManager(imageManager), nextModelId(1), frameId(1)
{
}

unsigned int ModelManager::LoadModel(const char* rootFilename)
{
    ModelLoader modelLoader(imageManager);

    TextureModel textureModel;
    if (!modelLoader.LoadModel(rootFilename, &textureModel))
    {
        Logger::Log("Error loading model '", rootFilename, "'.");
        return 0;
    }

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE1);
    GLuint mvMatrixImageId = imageManager->CreateEmptyTexture(MODEL_TEXTURE_SIZE, MODEL_TEXTURE_SIZE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE2);
    GLuint shadingColorAndSelectionImageId = imageManager->CreateEmptyTexture(MODEL_TEXTURE_SIZE, MODEL_TEXTURE_SIZE, GL_RGBA32F);\

    models.push_back(textureModel);
    dynamicRenderStore.push_back(TrackingRenderStore(mvMatrixImageId, shadingColorAndSelectionImageId));

    ++nextModelId;
    return nextModelId - 1;
}

const TextureModel& ModelManager::GetModel(unsigned int id)
{
    return models[id - 1];
}

// Retrieves the model ID given the name used to load the model, 0 if not found.
unsigned int ModelManager::GetModelId(std::string name) const
{
    for (unsigned int i = 0; i < models.size(); i++)
    {
        if (_stricmp(models[i].name.c_str(), name.c_str()) == 0)
        {
            return i + 1;
        }
    }

    return 0;
}

unsigned int ModelManager::GetCurrentModelCount() const
{
    return nextModelId;
}

void ModelManager::RenderModelImmediate(const glm::mat4& projectionMatrix, Model* model)
{
    glm::mat4 mvMatrix = glm::scale(BasicPhysics::GetBodyMatrix(model->body), model->scaleFactor);

    glUseProgram(directModelRenderProgram);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, models[model->modelId - 1].textureId);
    glUniform1i(directTextureLocation, 0);

    glUniform4f(directShadingColorLocation, model->color.x, model->color.y, model->color.z, model->color.w);
    glUniformMatrix4fv(directProjLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(directMvLocation, 1, GL_FALSE, &mvMatrix[0][0]);
    glUniform1f(directSelectionFactorLocation, model->selected ? 0.40f : 0.0f);

    glDrawElements(GL_TRIANGLES, models[model->modelId - 1].vertices.indices.size(), GL_UNSIGNED_INT, (const void*)(models[model->modelId - 1].indexOffset * sizeof(GL_UNSIGNED_INT)));
}

void ModelManager::RenderModel(const glm::mat4& projectionMatrix, Model* model)
{
    if (model->internalId == -1 || model->frameId != frameId - 1)
    {
        // Untracked. Add the model.
        AddNewModelToRenderStore(model);
    }
    else
    {
        // Known model. Only update if we detect it as dynamic.
        int activationState = model->analysisBody == nullptr ? model->body->getActivationState() : model->analysisBody->getActivationState();
        if (activationState == ACTIVE_TAG || activationState == DISABLE_DEACTIVATION || activationState == WANTS_DEACTIVATION)
        {
            UpdateModelInRenderStore(model);
        }
    }

    dynamicRenderStore[model->modelId - 1].renderedIds.insert(model->internalId);
    model->frameId = frameId;
}

void ModelManager::AddNewModelToRenderStore(Model* model)
{
    if (dynamicRenderStore[model->modelId - 1].freeIds.size() != 0)
    {
        model->internalId = *(dynamicRenderStore[model->modelId - 1].freeIds.begin());
        dynamicRenderStore[model->modelId - 1].freeIds.erase(dynamicRenderStore[model->modelId - 1].freeIds.begin());
    }
    else
    {
        model->internalId = dynamicRenderStore[model->modelId - 1].backingStore.GetInstanceCount();
    }

    UpdateModelInRenderStore(model);
}

void ModelManager::UpdateModelInRenderStore(Model* model)
{
    // Update in our backing store.
    dynamicRenderStore[model->modelId - 1].backingStore.InsertInModelStore(model->internalId, model);

    // Update in OpenGL.
    const ImageTexture& mvMatrixImage = imageManager->GetImage(dynamicRenderStore[model->modelId - 1].backingStore.mvMatrixImageId);
    const ImageTexture& shadingImage = imageManager->GetImage(dynamicRenderStore[model->modelId - 1].backingStore.shadingColorAndSelectionImageId);

    int x4 = (model->internalId * 4) % MODEL_TEXTURE_SIZE;
    int y4 = (model->internalId * 4) / MODEL_TEXTURE_SIZE;
    int x2 = (model->internalId * 2) % MODEL_TEXTURE_SIZE;
    int y2 = (model->internalId * 2) / MODEL_TEXTURE_SIZE;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mvMatrixImage.textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x4, y4, 4, 1, GL_RGBA, GL_FLOAT, &dynamicRenderStore[model->modelId - 1].backingStore.matrixStore[model->internalId * 4]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadingImage.textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x2, y2, 2, 1, GL_RGBA, GL_FLOAT, &dynamicRenderStore[model->modelId - 1].backingStore.shadingColorSelectionStore[model->internalId * 2]);
}

void ModelManager::ZeroIndex(unsigned int modelId, unsigned int idx)
{
    // TODO reconsider.
    // I'm not fixing this for now. Reasons:
    //  -- For now, it only appears in one frame.
    //  -- This is a performance hit, which so far it doesn't seem that I need. 
}

// Finalizes rendering (and actually renders) all models.
void ModelManager::FinalizeRender(const glm::mat4& projectionMatrix)
{
    ++frameId;

    glUseProgram(modelRenderProgram);
    glBindVertexArray(vao);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Always send dynamic data to the GPU, every frame.
    for (unsigned int i = 0; i < dynamicRenderStore.size(); i++)
    {
        // Remove what didn't render and zero it's data for reuse.
        for (unsigned int j = 0; j < dynamicRenderStore[i].backingStore.matrixStore.size(); j++)
        {
            if (dynamicRenderStore[i].freeIds.find(j) == dynamicRenderStore[i].freeIds.end() &&
                dynamicRenderStore[i].renderedIds.find(j) == dynamicRenderStore[i].renderedIds.end())
            {
                // Didn't render, zero it.
                dynamicRenderStore[i].freeIds.insert(j);
                ZeroIndex(i, j);
            }
        }

        if (dynamicRenderStore[i].backingStore.matrixStore.size() != 0)
        {
            // Bind everything; at this point, we've already sent everything to OpenGL.
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, models[i].textureId);
            glUniform1i(textureLocation, 0);

            int mvMatrixImageId = dynamicRenderStore[i].backingStore.mvMatrixImageId;
            int shadingImageId = dynamicRenderStore[i].backingStore.shadingColorAndSelectionImageId;
            const ImageTexture& mvMatrixImage = imageManager->GetImage(mvMatrixImageId);
            const ImageTexture& shadingImage = imageManager->GetImage(shadingImageId);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mvMatrixImage.textureId);
            glUniform1i(mvLocation, 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, shadingImage.textureId);
            glUniform1i(shadingColorLocation, 2);

            // Draw all the models of the specified type.
            glDrawElementsInstanced(GL_TRIANGLES, models[i].vertices.indices.size(), GL_UNSIGNED_INT,
                (const void*)(models[i].indexOffset * sizeof(GL_UNSIGNED_INT)), dynamicRenderStore[i].backingStore.GetInstanceCount());
        }
    }
}

// Initializes the OpenGL resources
bool ModelManager::InitializeOpenGlResources(ShaderManager& shaderManager)
{
    if (!shaderManager.CreateShaderProgram("modelRender", &modelRenderProgram))
    {
        Logger::LogError("Could not create the model shader!");
        return false;
    }

    textureLocation = glGetUniformLocation(modelRenderProgram, "modelTexture");
    mvLocation = glGetUniformLocation(modelRenderProgram, "mvMatrix");
    projLocation = glGetUniformLocation(modelRenderProgram, "projMatrix");
    shadingColorLocation = glGetUniformLocation(modelRenderProgram, "shadingColorAndFactor");

    if (!shaderManager.CreateShaderProgram("directModelRender", &directModelRenderProgram))
    {
        Logger::LogError("Could not create the direct model shader!");
        return false;
    }

    directTextureLocation = glGetUniformLocation(directModelRenderProgram, "modelTexture");
    directMvLocation = glGetUniformLocation(directModelRenderProgram, "mvMatrix");
    directProjLocation = glGetUniformLocation(directModelRenderProgram, "projMatrix");
    directShadingColorLocation = glGetUniformLocation(directModelRenderProgram, "shadingColor");
    directSelectionFactorLocation = glGetUniformLocation(directModelRenderProgram, "selectionFactor");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &positionBuffer);
    glGenBuffers(1, &uvBuffer);
    glGenBuffers(1, &indexBuffer);

    return true;
}

// Finalizes the list of loaded models we know of, sending the data to OpenGL.
void ModelManager::FinalizeLoadedModels()
{
    glBindVertexArray(vao);

    universalVertices temporaryCopyVertices;

    unsigned int indexPositionReferralOffset = 0;
    unsigned int indexOffset = 0;
    for (unsigned int i = 0; i < models.size(); i++)
    {
        models[i].indexOffset = indexOffset;
        temporaryCopyVertices.positions.insert(temporaryCopyVertices.positions.end(), models[i].vertices.positions.begin(), models[i].vertices.positions.end());
        temporaryCopyVertices.uvs.insert(temporaryCopyVertices.uvs.end(), models[i].vertices.uvs.begin(), models[i].vertices.uvs.end());

        for (unsigned int j = 0; j < models[i].vertices.indices.size(); j++)
        {
            temporaryCopyVertices.indices.push_back(models[i].vertices.indices[j] + indexPositionReferralOffset);
        }

        indexPositionReferralOffset += models[i].vertices.positions.size();
        indexOffset += models[i].vertices.indices.size();
    }

    temporaryCopyVertices.TransferStaticPositionToOpenGl(positionBuffer);
    temporaryCopyVertices.TransferStaticUvsToOpenGl(uvBuffer);
    temporaryCopyVertices.TransferStaticIndicesToOpenGl(indexBuffer);
}

// Deletes all initialized OpenGL resources.
ModelManager::~ModelManager()
{
    glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &positionBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &indexBuffer);
}
