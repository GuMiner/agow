#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>

struct RenderableSentence
{
    int sentenceId;
    glm::mat4 posRotMatrix;
    glm::vec3 color;
};