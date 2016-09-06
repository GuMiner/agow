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
    if (!shaderManager.CreateShaderProgram("starRender", &starProgram))
    {
        Logger::Log("Failure creating the sky cube shader program!");
        return false;
    }

    projMatrixLocation = glGetUniformLocation(starProgram, "projMatrix");
    flavorColorLocation = glGetUniformLocation(starProgram, "flavorColor");

    // We need the VAO to actually render without sending any vertex data to the shader, which handles the stars.
    glGenVertexArrays(1, &starVao);
    glBindVertexArray(starVao);

    return true;
}

void Scenery::UpdateSkyColoration(vec::vec3 skyColor, float strength)
{
    flavorColor = skyColor;
    flavorColorStrength = strength;
}

void Scenery::Render(vec::mat4& projectionMatrix)
{
    // Render the sky
    glUseProgram(starProgram);
    glBindVertexArray(starVao);
    
    glUniform4f(flavorColorLocation, flavorColor.x, flavorColor.y, flavorColor.z, flavorColorStrength);
    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, projectionMatrix);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

Scenery::~Scenery()
{
    glDeleteVertexArrays(1, &starVao);
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
        vec::vec3 randomColor = vec::vec3(0.50f + 0.90f * MathOps::Rand(), 0.50f + 0.90f * MathOps::Rand(), 0.50f + 0.90f * MathOps::Rand());
        float strength = 1.0f;
        UpdateSkyColoration(randomColor, strength);
    }
}
