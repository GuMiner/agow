#include <string>
#include <sstream>
#include <glm/gtc/random.hpp>
#include "Utils\ImageUtils.h"
#include "Utils\Logger.h"
#include "Scenery.h"

Scenery::Scenery()
    : flavorColor(glm::vec3(1.0f, 1.0f, 1.0f)), flavorColorStrength(1.0f)
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

void Scenery::UpdateSkyColoration(glm::vec3 skyColor, float strength)
{
    flavorColor = skyColor;
    flavorColorStrength = strength;
}

void Scenery::Render(glm::mat4& projectionMatrix)
{
    // Render the sky
    glUseProgram(starProgram);
    glBindVertexArray(starVao);
    
    glUniform4f(flavorColorLocation, flavorColor.x, flavorColor.y, flavorColor.z, flavorColorStrength);
    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

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
        glm::vec3 randomColor = glm::vec3(0.50f) + glm::linearRand(glm::vec3(0.0f), glm::vec3(0.90f));
        float strength = 1.0f;
        UpdateSkyColoration(randomColor, strength);
    }
}
