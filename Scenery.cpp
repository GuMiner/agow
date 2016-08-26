#include <string>
#include <sstream>
#include "Math\MatrixOps.h"
#include "Math\MathOps.h"
#include "Utils\ImageUtils.h"
#include "Utils\Logger.h"
#include "Scenery.h"

Scenery::Scenery()
    : flavorColor(vec::vec3(1.0f, 1.0f, 1.0f)), flavorColorStrength(1.0f)
{
}

bool Scenery::Initialize(ShaderManager& shaderManager)
{
    // Sky program.
    if (!shaderManager.CreateShaderProgram("skyCubeRender", &skyCubeProgram))
    {
        Logger::Log("Failure creating the sky cube shader program!");
        return false;
    }

    viewMatrixLocation = glGetUniformLocation(skyCubeProgram, "viewMatrix");
    flavorColorLocation = glGetUniformLocation(skyCubeProgram, "flavorColor");

    // We need the VAO to actually render without sending any vertex data to the shader, which handles the stars.
    glGenVertexArrays(1, &skyCubeVao);
    glBindVertexArray(skyCubeVao);

    return true;
}

void Scenery::UpdateSkyColoration(vec::vec3 skyColor, float strength)
{
    flavorColor = skyColor;
    flavorColorStrength = strength;
}

void Scenery::Render(vec::mat4& viewMatrix)
{
    // Render the sky
    glUseProgram(skyCubeProgram);
    glBindVertexArray(skyCubeVao);

    glUniform4f(flavorColorLocation, flavorColor.x, flavorColor.y, flavorColor.z, flavorColorStrength);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_TRUE, viewMatrix);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

Scenery::~Scenery()
{
    glDeleteVertexArrays(1, &skyCubeVao);
}

void Scenery::Callback(EventType eventType, void* callbackSpecificData)
{
    if (eventType == EventType::SKY_FLAVOR_COLOR_CHANGE && callbackSpecificData != nullptr)
    {
        SkyColorChange* colorChange = (SkyColorChange*)callbackSpecificData;
        UpdateSkyColoration(colorChange->flavorColor, colorChange->strength);
    }
    else if (eventType == EventType::SKY_FLAVOR_RANDOM_COLOR_CHANGE)
    {
        // Used for fancy effects, not for in-game 'mood' determinations. TODO configurable.
        vec::vec3 randomColor = vec::vec3(0.70f + 0.20f * MathOps::Rand(), 0.70f + 0.20f * MathOps::Rand(), 0.70f + 0.20f * MathOps::Rand());
        float strength = 0.60f + MathOps::Rand() * 2.0f;
        UpdateSkyColoration(randomColor, strength);
    }
}
