#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include "Managers\ModelManager.h"

class Vehicle
{
    // Loads components necessary for the vehicle to render.
    virtual bool LoadVehicleComponents(ModelManager* modelManager) = 0;
    
    // Gets the point that's considered for user interaction.
    virtual glm::vec3 GetInteractionPoint() const = 0;
    
    // Updates the vehicle from time effects.
    virtual void Update(float gameTime, float frameTime) = 0;
    
    // Updates the vehicle from user input.
    virtual void UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide) = 0;
    
    // Renders all components of the vehicle.
    virtual void Render(ModelManager* modelManager, const glm::mat4& projectionMatrix) = 0;
};