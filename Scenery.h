#pragma once
#include <GL\glew.h>
#include <glm\vec3.hpp>
#include "Data\CallbackTypes.h"
#include "Managers\ModelManager.h"
#include "Managers\ShaderManager.h"
#include "Utils\TypedCallback.h"

// Renders long-range scenery to give an impression of an infinite world.
class Scenery : public ICallback<EventType>
{
    // Sky cube
    GLuint starProgram;
    GLuint projMatrixLocation;
    GLuint flavorColorLocation;

    GLuint starVao;

    float flavorColorStrength;
    glm::vec3 flavorColor;

    void UpdateSkyColoration(glm::vec3 skyColor, float strength);
public:
    Scenery();

    bool Initialize(ShaderManager& shaderManager);
    void Render(glm::mat4& projectionMatrix);

    // Used for sky coloration changes.
    virtual void Callback(EventType eventType, void* callbackSpecificData) override;

    virtual ~Scenery();
};

