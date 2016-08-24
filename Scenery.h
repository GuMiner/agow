#pragma once
#include <GL\glew.h>
#include "Managers\ModelManager.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

// Renders long-range scenery to give an impression of an infinite world.
class Scenery
{
    // Sky cube
    GLuint skyCubeProgram;
    GLuint viewMatrixLocation;
    GLuint flavorColorLocation;

    GLuint skyCubeVao;

    float flavorColorStrength;
    vec::vec3 flavorColor;

public:
    Scenery();

    bool Initialize(ShaderManager& shaderManager);
    void UpdateSkyColoration(vec::vec3 skyColor, float strength);
    void Render(vec::mat4& viewMatrix);

    virtual ~Scenery();
};

