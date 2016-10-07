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

struct PhysicalModel
{
    unsigned int modelId;
    btRigidBody* rigidBody;
};

struct ScaledPhysicalModel
{
    // We scale drawing by this factor. The rigid body is assumed to already be scaled.
    glm::vec3 scaleFactor;
    unsigned int modelId;
    btRigidBody* rigidBody;
};

struct ColoredPhysicalModel
{
    glm::vec4 color;
    PhysicalModel model;
};

struct PhysicalModelSet
{
    unsigned int modelId;
    std::vector<btRigidBody*> rigidBodies;
};