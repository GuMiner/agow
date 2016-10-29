#pragma once

// List of vertex types that are sendable to OpenGL
#include <vector>
#include <GL/glew.h>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>

struct universalVertices
{
private:
    // Sends floating-point vec2/3/4 data to OpenGL
    template<typename T>
    static void SendToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, const std::vector<T>& data);

    template<typename T>
    static void SendToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, GLenum usage, const std::vector<T>& data);

    // Sends indexed unsigned integer index data to OpenGL
    static void SendUIntToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, const std::vector<unsigned int>& data);

    static void SendUIntToOpenGl(GLuint buffer, GLuint shaderIdx, GLuint itemCount, GLenum usage, const std::vector<unsigned int>& data);

    // Sends indicies for indexed drawing to OpenGL
    static void SendIndicesToOpenGl(GLuint buffer, const std::vector<unsigned int>& data);

public:

    // Note that each item here follows the given buffer layout.
    std::vector<glm::vec3> positions; // layout position 0
    std::vector<glm::vec3> colors;
    std::vector<glm::vec4> barycentrics;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> ids;

    std::vector<unsigned int> indices;

    // Clears all the data in the universal vertices, minus IDs.
    void Reset();

    // Adds a position, color, UV vertex.
    void AddColorTextureVertex(glm::vec3 position, glm::vec3 color, glm::vec2 uv);

    // Transfers the vertex data for the specified array to OpenGL.
    void TransferPositionToOpenGl(GLuint positionBuffer);
    void TransferColorToOpenGl(GLuint colorBuffer);
    
    void TransferUpdatePositionToOpenGl(GLuint positionBuffer);
    void TransferStaticPositionToOpenGl(GLuint positionBuffer);
    void TransferStaticColorToOpenGl(GLuint colorBuffer);
    
    void TransferBarycentricsToOpenGl(GLuint barycentricBuffer);
    void TransferUvsToOpenGl(GLuint uvBuffer);
    void TransferIdsToOpenGl(GLuint idBuffer);
    void TransferIndicesToOpenGl(GLuint indiciesBuffer);
    void TransferStaticUvsToOpenGl(GLuint uvBuffer);
    void TransferStaticIdsToOpenGl(GLuint idBuffer);
    void TransferStaticIndicesToOpenGl(GLuint indiciesBuffer);

    // Transfers a vec4 position set directly to OpenGL. Used for *speed* within asteroid rendering for LOD effects.
    static void TransferDirectToOpenGl(const std::vector<glm::vec4>& positions, GLuint positionBuffer, const std::vector<glm::vec3>& colors, GLuint colorBuffer);
};

struct DrawArraysIndirectCommand
{
    GLuint vertexCount;
    GLuint instanceCount;
    GLuint firstVertexOffset;
    GLuint baseInstance;

    void Set(GLuint vertexCount, GLuint instanceCount, GLuint firstVertexOffset, GLuint baseInstance);
};
