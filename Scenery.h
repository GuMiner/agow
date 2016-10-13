#pragma once
#include <GL\glew.h>
#include <glm\vec3.hpp>
#include "Data\CallbackTypes.h"
#include "Managers\ModelManager.h"
#include "Managers\ShaderManager.h"
#include "Utils\TypedCallback.h"

struct Stars
{
    GLuint programId;
    GLuint projMatrixLocation;
    GLuint flavorColorLocation;

    GLuint vao;

    float flavorColorStrength;
    glm::vec3 flavorColor;
};

struct Cloud
{
    glm::vec3 position;

    GLuint vao;
    GLuint positionBuffer;
    unsigned int drawIdBuffer;
    universalVertices cloud;
};

struct Clouds
{
    GLuint programId;
    GLuint projMatrixLocation;
    GLuint mvMatrixLocation;
    
    std::vector<Cloud> clouds;
};

// Renders long-range scenery to give an impression of an infinite world.
class Scenery : public ICallback<EventType>
{
    Stars stars;
    Clouds clouds;

    void UpdateSkyColoration(glm::vec3 skyColor, float strength);
public:
    Scenery();

    bool Initialize(ShaderManager& shaderManager);
    void Update(float frameTime);
    void Render(glm::mat4& projectionMatrix, const glm::vec3& playerPosition);

    // Used for sky coloration changes.
    virtual void Callback(EventType eventType, void* callbackSpecificData) override;

    virtual ~Scenery();
};

