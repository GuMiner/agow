#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>
#include "Data\Model.h"

class ModelRenderStore
{
public:

    // These are created from the image manager and do not need to be freed.
    GLuint mvMatrixImageId;
    GLuint shadingColorAndSelectionImageId;

    // Stores MV matrices, split into 4 vectors per matrix.
    std::vector<glm::vec4> matrixStore;

    // Stores the shading color (even indices) and selection bool ('R', odd incies)
    std::vector<glm::vec4> shadingColorSelectionStore;

    ModelRenderStore(GLuint mvMatrixImageId, GLuint shadingColorAndSelectionImageId);
    unsigned int GetInstanceCount();
    void AddModelToStore(Model* model);
    void InsertInModelStore(unsigned int location, Model* model);
    void Clear();
};