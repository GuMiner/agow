#include <glm\vec2.hpp>
#include "Vertex.h"

template<typename T>
void universalVertices::SendToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, const std::vector<T>& data)
{
    glEnableVertexAttribArray(shaderIdx);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(shaderIdx, itemCount, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(T), &data[0], GL_DYNAMIC_DRAW);
}

template<typename T>
void universalVertices::SendToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, GLenum usage, const std::vector<T>& data)
{
    glEnableVertexAttribArray(shaderIdx);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(shaderIdx, itemCount, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(T), &data[0], usage);
}

void universalVertices::SendUIntToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, const std::vector<unsigned int>& data)
{
    glEnableVertexAttribArray(shaderIdx);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    // Note this is glVertexAttrib*I*Pointer in comparison to the other call above.
    glVertexAttribIPointer(shaderIdx, itemCount, GL_UNSIGNED_INT, 0, nullptr);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(unsigned int), &data[0], GL_DYNAMIC_DRAW);
}

void universalVertices::SendUIntToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, GLenum usage, const std::vector<unsigned int>& data)
{
    glEnableVertexAttribArray(shaderIdx);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    // Note this is glVertexAttrib*I*Pointer in comparison to the other call above.
    glVertexAttribIPointer(shaderIdx, itemCount, GL_UNSIGNED_INT, 0, nullptr);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(unsigned int), &data[0], usage);
}

void universalVertices::SendIndicesToOpenGl(GLuint buffer, const std::vector<unsigned int>& data)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), &data[0], GL_DYNAMIC_DRAW);
}

void universalVertices::Reset()
{
    positions.clear();
    colors.clear();
    barycentrics.clear();
    uvs.clear();
    ids.clear();
    indices.clear();
}

void universalVertices::AddColorTextureVertex(glm::vec3 position, glm::vec3 color, glm::vec2 uv)
{
    positions.push_back(position);
    colors.push_back(color);
    uvs.push_back(uv);
}

void universalVertices::TransferUpdatePositionToOpenGl(GLuint positionBuffer)
{
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size()*sizeof(glm::vec3), &positions[0]);
}

void universalVertices::TransferPositionToOpenGl(GLuint positionBuffer)
{
    universalVertices::SendToOpenGl(positionBuffer, 0, 3, positions);
}

void universalVertices::TransferColorToOpenGl(GLuint colorBuffer)
{
    universalVertices::SendToOpenGl(colorBuffer, 1, 3, colors);
}

void universalVertices::TransferStaticPositionToOpenGl(GLuint positionBuffer)
{
    universalVertices::SendToOpenGl(positionBuffer, 0, 3, GL_STATIC_DRAW, positions);
}

void universalVertices::TransferStaticColorToOpenGl(GLuint colorBuffer)
{
    universalVertices::SendToOpenGl(colorBuffer, 1, 3, GL_STATIC_DRAW, colors);
}

void universalVertices::TransferBarycentricsToOpenGl(GLuint barycentricBuffer)
{
    universalVertices::SendToOpenGl(barycentricBuffer, 2, 4, barycentrics);
}

void universalVertices::TransferUvsToOpenGl(GLuint uvBuffer)
{
    universalVertices::SendToOpenGl(uvBuffer, 3, 2, uvs);
}

void universalVertices::TransferIdsToOpenGl(GLuint idBuffer)
{
    universalVertices::SendUIntToOpenGl(idBuffer, 4, 1, ids);
}

void universalVertices::TransferIndicesToOpenGl(GLuint indiciesBuffer)
{
    universalVertices::SendIndicesToOpenGl(indiciesBuffer, indices);
}

void universalVertices::TransferStaticUvsToOpenGl(GLuint uvBuffer)
{
    universalVertices::SendToOpenGl(uvBuffer, 3, 2, GL_STATIC_DRAW, uvs);
}

void universalVertices::TransferStaticIdsToOpenGl(GLuint idBuffer)
{
    universalVertices::SendUIntToOpenGl(idBuffer, 4, 1, GL_STATIC_DRAW, ids);
}

void universalVertices::TransferStaticIndicesToOpenGl(GLuint indiciesBuffer)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiciesBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void universalVertices::TransferDirectToOpenGl(const std::vector<glm::vec4>& positions, GLuint positionBuffer, const std::vector<glm::vec3>& colors, GLuint colorBuffer)
{
    universalVertices::SendToOpenGl(positionBuffer, 0, 4, positions);
    universalVertices::SendToOpenGl(colorBuffer, 1, 3, colors);

}

void DrawArraysIndirectCommand::Set(GLuint vertexCount, GLuint instanceCount, GLuint firstVertexOffset, GLuint baseInstance)
{
    this->vertexCount = vertexCount;
    this->instanceCount = instanceCount;
    this->firstVertexOffset = firstVertexOffset;
    this->baseInstance = baseInstance;
}
