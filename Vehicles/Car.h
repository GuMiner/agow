#pragma once
#include "BasicPhysics.h"
#include "Vehicle.h"

class Car : public Vehicle
{
    void SetupPhysics(BasicPhysics* physics);

    // Inherited via Vehicle
    virtual bool LoadVehicleComponents(ModelManager * modelManager) override;
    virtual void Update(float gameTime, float frameTime) override;
    virtual void UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide) override;
    virtual void Render(ModelManager * modelManager, const glm::mat4 & projectionMatrix) override;
    virtual glm::vec3 GetInteractionPoint() const override;
};