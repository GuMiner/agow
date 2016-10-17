#pragma once
#include "Vehicle.h"

class Tank : public Vehicle
{
    // Inherited via Vehicle
    virtual bool LoadVehicleComponents(ModelManager * modelManager) override;
    virtual void Update(float gameTime, float frameTime) override;
    virtual void UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide) override;
    virtual void Render(ModelManager * modelManager, const glm::mat4 & projectionMatrix) override;
    virtual glm::vec3 GetInteractionPoint() const override;
};