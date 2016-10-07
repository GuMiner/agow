#pragma once
#include <glm\vec3.hpp>

struct RenderableSentence
{
    int sentenceId;
    glm::mat4 posRotMatrix;
    glm::vec3 color;
};
