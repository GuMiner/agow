#pragma once
#include <vector>
#include <Bullet\btBulletDynamicsCommon.h>
#include <GL\glew.h>
#include "Utils\Vertex.h"
#include "Math\Vec.h"

// Holds data from a loaded, indexed UV textured model.
struct TextureModel
{
    // Texture to apply to the model.
    GLuint textureId;

    // Vertex data -- only the position, uvs, and indices fields are expected to be filled.
    universalVertices vertices;

    // Offset to apply to all the indicies in this model when sending to OpenGL and rendering.
    GLuint indexOffset;

    // Model bounding box.
    vec::vec3 minBounds;
    vec::vec3 maxBounds;
};

struct PhysicalModel
{
    unsigned int modelId;
    btRigidBody* rigidBody;
};

struct PhysicalModelSet
{
    unsigned int modelId;
    std::vector<btRigidBody*> rigidBodies;
};