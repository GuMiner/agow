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
    GLuint mvMatrixImageIdStatic = imageManager->CreateEmptyTexture(MODEL_TEXTURE_SIZE, MODEL_TEXTURE_SIZE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE2);
    GLuint shadingColorAndSelectionImageId = imageManager->CreateEmptyTexture(MODEL_TEXTURE_SIZE, MODEL_TEXTURE_SIZE, GL_RGBA32F);
    GLuint shadingColorAndSelectionImageIdStatic = imageManager->CreateEmptyTexture(MODEL_TEXTURE_SIZE, MODEL_TEXTURE_SIZE, GL_RGBA32F);

    models.push_back(textureModel);
    dynamicRenderStore.push_back(ModelRenderStore(mvMatrixImageId, shadingColorAndSelectionImageId));
    staticRenderStore.push_back(StaticRenderStore(mvMatrixImageIdStatic, shadingColorAndSelectionImageIdStatic));

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
    int activationState = model->analysisBody == nullptr ? model->body->getActivationState() : model->analysisBody->getActivationState();
    if (activationState == ACTIVE_TAG || activationState == DISABLE_DEACTIVATION || activationState == WANTS_DEACTIVATION)
    {
        RenderDynamicModel(projectionMatrix, model);
    }
    
    RenderStaticModel(model);
}

void ModelManager::AddNewStaticModel(Model* model)
{
    StaticRenderStore& renderStore = staticRenderStore[model->modelId - 1];

    // Find first free spot and set that as the internal ID.
    if (renderStore.drawSegments.size() == 0)
    {
        // No items at all. This item takes first place.
        model->internalId = 1;
        renderStore.drawSegments.push_back(glm::ivec2(0, 1));
    }
    else
    {
        // Add to the existing list.
        model->internalId = staticRenderStore[model->modelId - 1].drawSegments.front().y + 1;
        renderStore.drawSegments.front().y++;

        if (renderStore.drawSegments.size() > 1)
        {
            glm::ivec2 nextElement = *(renderStore.drawSegments.begin()++);
            if (nextElement.x == renderStore.drawSegments.front().y)
            {
                // The next segment starts when this segment now ends. Extend that segment and remove the front segment.
                nextElement.x = 0;
                renderStore.drawSegments.pop_front();
            }
        }
    }

    // Using that ID, save the data into the backing store.
    renderStore.backingStore.InsertInModelStore(model->internalId - 1, model);
    renderStore.newItemsAdded.push_back(model->internalId - 1);
}

void ModelManager::RenderStaticModel(Model* model)
{
    if (model->internalId == -1 || model->frameId != frameId - 1)
    {
        // We consider this a new static model if the internal ID is zero or the frame ID is not from the last frame.
        AddNewStaticModel(model);
    }

    staticRenderStore[model->modelId - 1].drawnItems.insert(model->internalId - 1);

    // If someone removes a static item from rendering and adds it back in next frame, the frame IDs will differ.
    // Storing the frame ID ensures our ID can be the ID in the static element array and not something more complicated.
    model->frameId = frameId;
}

// Renders the specified model given by the ID, using the given color.
void ModelManager::RenderDynamicModel(const glm::mat4& projectionMatrix, Model* model)
{
    RenderModelImmediate(projectionMatrix, model);

    // This is too slow, the matrices are too large! We also don't have that much dynamic stuff per-frame.
    // model->internalId = -1;
    // dynamicRenderStore[model->modelId - 1].AddModelToStore(model);
}

// Finalizes rendering (and actually renders) all models.
void ModelManager::FinalizeRender(const glm::mat4& projectionMatrix)
{
    ++frameId;

    glUseProgram(modelRenderProgram);
    glBindVertexArray(vao);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    FinalizeDynamicRender();
    FinalizeStaticRender();
}

void ModelManager::FinalizeDynamicRender()
{
    glUniform1i(instanceOffsetLocation, 0);

    // Always send dynamic data to the GPU, every frame.
    for (unsigned int i = 0; i < dynamicRenderStore.size(); i++)
    {
        if (dynamicRenderStore[i].matrixStore.size() != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, models[i].textureId);
            glUniform1i(textureLocation, 0);

            // Send model data to OpenGL
            int mvMatrixImageId = dynamicRenderStore[i].mvMatrixImageId;
            int shadingImageId = dynamicRenderStore[i].shadingColorAndSelectionImageId;
            const ImageTexture& mvMatrixImage = imageManager->GetImage(mvMatrixImageId);
            const ImageTexture& shadingImage = imageManager->GetImage(shadingImageId);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mvMatrixImage.textureId);
            std::memcpy(mvMatrixImage.imageData, &(dynamicRenderStore[i].matrixStore)[0], dynamicRenderStore[i].matrixStore.size() * sizeof(glm::vec4));
            imageManager->ResendToOpenGl(mvMatrixImageId);
            glUniform1i(mvLocation, 1);

            // Send shading color and selection data to OpenGL
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, shadingImage.textureId);
            std::memcpy(shadingImage.imageData, &(dynamicRenderStore[i].shadingColorSelectionStore)[0], dynamicRenderStore[i].shadingColorSelectionStore.size() * sizeof(glm::vec4));
            imageManager->ResendToOpenGl(shadingImageId);
            glUniform1i(shadingColorLocation, 2);

            // Draw all the models of the specified type.
            glDrawElementsInstanced(GL_TRIANGLES, models[i].vertices.indices.size(), GL_UNSIGNED_INT,
                (const void*)(models[i].indexOffset * sizeof(GL_UNSIGNED_INT)), dynamicRenderStore[i].GetInstanceCount());

            // Clear the dynamic store.
            dynamicRenderStore[i].Clear();
        }
    }
}

void ModelManager::FinalizeStaticRender()
{
    for (unsigned int i = 0; i < staticRenderStore.size(); i++)
    {
        if (staticRenderStore[i].drawSegments.size() != 0)
        {
            // Remove items that didn't render.
            for (auto iter = staticRenderStore[i].drawSegments.begin(); iter != staticRenderStore[i].drawSegments.end(); iter++)
            {
                for (int j = iter->x; j < iter->x + iter->y; j++)
                {
                    if (staticRenderStore[i].drawnItems.find(j) == staticRenderStore[i].drawnItems.end())
                    {
                        if (j == iter->x)
                        {
                            // We're at the start, so just push the range up.
                            iter->x++;
                            iter->y--;
                            if (iter->y == 0)
                            {
                                // Remove this element
                                iter = staticRenderStore[i].drawSegments.erase(iter);
                                j = iter->x - 1;
                            }

                            continue;
                        }
                        else if (j == iter->x + iter->y - 1)
                        {
                            // We're at the end, so just pull the range down.
                            iter->y--;
                            if (iter->y == 0)
                            {
                                // Remove this element
                                iter = staticRenderStore[i].drawSegments.erase(iter);
                                j = iter->x - 1;
                            }

                            continue;
                        }
                        else
                        {
                            // This is an element inside the iterator. First, find what the new segment will be.
                            glm::ivec2 nextSegment = glm::ivec2(j + 1, iter->y - (j + 1 - iter->x));

                            // Now cut the current segment short.
                            iter->y = j - iter->x;
                            
                            // Now add the new segment.
                            iter++; // Item added before the current item.
                            iter = staticRenderStore[i].drawSegments.insert(iter, nextSegment);
                            iter--;
                        }
                    }
                }
            }

            staticRenderStore[i].drawnItems.clear();

            const ImageTexture& mvMatrixImage = imageManager->GetImage(staticRenderStore[i].backingStore.mvMatrixImageId);
            const ImageTexture& shadingImage = imageManager->GetImage(staticRenderStore[i].backingStore.shadingColorAndSelectionImageId);

            // Add items that were added for rendering.
            if (true)//staticRenderStore[i].newItemsAdded.size() > 2000)
            {
                // Just replace the whole thing.
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mvMatrixImage.textureId);
                std::memcpy(mvMatrixImage.imageData, &(staticRenderStore[i].backingStore.matrixStore)[0], staticRenderStore[i].backingStore.matrixStore.size() * sizeof(glm::vec4));
                imageManager->ResendToOpenGl(staticRenderStore[i].backingStore.mvMatrixImageId);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, shadingImage.textureId);
                std::memcpy(shadingImage.imageData, &(dynamicRenderStore[i].shadingColorSelectionStore)[0], dynamicRenderStore[i].shadingColorSelectionStore.size() * sizeof(glm::vec4));
                imageManager->ResendToOpenGl(staticRenderStore[i].backingStore.shadingColorAndSelectionImageId);
            }
            else
            {
                // Do an itemized replacement. TODO
            }

            // Bind all textures, but don't copy anything as that's already been done.
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, models[i].textureId);
            glUniform1i(textureLocation, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mvMatrixImage.textureId);
            glUniform1i(mvLocation, 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, shadingImage.textureId);
            glUniform1i(shadingColorLocation, 2);

            // Render the remainder.
            for (glm::ivec2& renderPair : staticRenderStore[i].drawSegments)
            {
                // Offset at a specified instance.
                glUniform1i(instanceOffsetLocation, renderPair.x);

                // Render all instances listed.
                glDrawElementsInstanced(GL_TRIANGLES, models[i].vertices.indices.size(), GL_UNSIGNED_INT,
                    (const void*)(models[i].indexOffset * sizeof(GL_UNSIGNED_INT)), renderPair.y);
            }
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

    instanceOffsetLocation = glGetUniformLocation(modelRenderProgram, "instanceOffset");
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

    temporaryCopyVertices.TransferPositionToOpenGl(positionBuffer);
    temporaryCopyVertices.TransferUvsToOpenGl(uvBuffer);
    temporaryCopyVertices.TransferIndicesToOpenGl(indexBuffer);
}

// Deletes all initialized OpenGL resources.
ModelManager::~ModelManager()
{
    glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &positionBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &indexBuffer);
}
