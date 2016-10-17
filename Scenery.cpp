#include <string>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include "Utils\ImageUtils.h"
#include "Utils\Logger.h"
#include "Scenery.h"

Scenery::Scenery()
{
    stars.flavorColor = glm::vec3(1.0f, 1.0f, 1.0f);
    stars.flavorColorStrength = 1.0f;
}

bool Scenery::Initialize(ShaderManager& shaderManager)
{
    // Sky program.
    if (!shaderManager.CreateShaderProgram("starRender", &stars.programId))
    {
        Logger::Log("Failure creating the sky cube shader program!");
        return false;
    }

    stars.projMatrixLocation = glGetUniformLocation(stars.programId, "projMatrix");
    stars.flavorColorLocation = glGetUniformLocation(stars.programId, "flavorColor");

    // We need the VAO to actually render without sending any vertex data to the shader, which handles the stars.
    glGenVertexArrays(1, &stars.vao);
    glBindVertexArray(stars.vao);

    // Clouds program.
    if (!shaderManager.CreateShaderProgram("cloudRender", &clouds.programId))
    {
        Logger::Log("Failure creating the cloud shader program!");
        return false;
    }

    clouds.projMatrixLocation = glGetUniformLocation(clouds.programId, "projMatrix");
    clouds.mvMatrixLocation = glGetUniformLocation(clouds.programId, "mvMatrix");

    // TODO configurable amount of clouds
    for (int i = 0; i < 10; i++)
    {

    }

    return true;
}

void Scenery::Update(float frameTime)
{
    // TODO move the clouds around.
}

void Scenery::UpdateSkyColoration(glm::vec3 skyColor, float strength)
{
    stars.flavorColor = skyColor;
    stars.flavorColorStrength = strength;
}

void Scenery::Render(glm::mat4& projectionMatrix, const glm::vec3& playerPosition)
{
    // Render the stars
    glUseProgram(stars.programId);
    glBindVertexArray(stars.vao);
    
    glUniform4f(stars.flavorColorLocation, stars.flavorColor.x, stars.flavorColor.y, stars.flavorColor.z, stars.flavorColorStrength);
    glUniformMatrix4fv(stars.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    // 12 == cube.
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Render the clouds
    glUseProgram(clouds.programId);
    glUniformMatrix4fv(clouds.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    for (unsigned int i = 0; i < clouds.clouds.size(); i++)
    {
        glm::mat4 mvMatrix = glm::translate(glm::translate(glm::mat4(), playerPosition), clouds.clouds[i].position);
        glUniformMatrix4fv(clouds.mvMatrixLocation, 1, GL_FALSE, &mvMatrix[0][0]);
        
        glBindVertexArray(clouds.clouds[i].vao);
        glDrawArrays(GL_POINTS, 0, clouds.clouds[i].cloud.positions.size());
    }
}

Scenery::~Scenery()
{
    glDeleteVertexArrays(1, &stars.vao);
    glDeleteProgram(stars.programId);

    glDeleteProgram(clouds.programId);
    for (unsigned int i = 0; i < clouds.clouds.size(); i++)
    {
        glDeleteBuffers(1, &clouds.clouds[i].positionBuffer);
        glDeleteBuffers(1, &clouds.clouds[i].drawIdBuffer);
        glDeleteVertexArrays(1, &clouds.clouds[i].vao);
    }
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
