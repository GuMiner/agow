#pragma once
#include <string>
#include <vector>
#include <Bullet\btBulletDynamicsCommon.h>
#include <GL\glew.h>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include "Utils\Vertex.h"

// Holds data from a loaded, indexed UV textured model.
struct TextureModel
{
    // Name used to load the model.
    std::string name;

    // Texture to apply to the model.
    GLuint textureId;

    // Total number of points before remapping points were added to the end. Used to grab points for physics calculations
    int rawPointCount;

    // Vertex data -- only the position, uvs, and indices fields are expected to be filled.
    universalVertices vertices;

    // Offset to apply to all the indicies in this model when sending to OpenGL and rendering.
    GLuint indexOffset;

    // Model bounding box.
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
};

struct Model
{
    // Used internally to speed up drawing operations.
    int internalId;
    long frameId;

    // The physical body to use for analysis of this object.
    // For example, the 'analysisBody' of a building segment is the entire building, 
    //  until the building is interacted with, at which point the 'analysisBody' == 'body'.
    btRigidBody* analysisBody;

    unsigned int modelId;
    btRigidBody* body;

    // Modifications to drawing for the item.
    glm::vec3 scaleFactor;
    glm::vec4 color;
    bool selected;

    Model()
        : internalId(-1), frameId(0), analysisBody(nullptr), body(nullptr), modelId(0), color(1.0f), scaleFactor(1.0f), selected(false)
    {
    }
};