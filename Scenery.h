#pragma once
#include <GL\glew.h>
#include "Data\CallbackTypes.h"
#include "Managers\ModelManager.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"
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
    vec::vec3 flavorColor;

    void UpdateSkyColoration(vec::vec3 skyColor, float strength);
public:
    Scenery();

    bool Initialize(ShaderManager& shaderManager);
    void Render(vec::mat4& projectionMatrix);

    // Used for sky coloration changes.
    virtual void Callback(EventType eventType, void* callbackSpecificData) override;

    virtual ~Scenery();
};

