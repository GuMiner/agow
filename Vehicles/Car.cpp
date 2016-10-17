#include "Car.h"

bool Car::LoadVehicleComponents(ModelManager* modelManager)
{
    return false;
}

void Car::SetupPhysics(BasicPhysics* physics)
{
    // physics->GetDynamicBody()
}

void Car::Update(float gameTime, float frameTime)
{
}

void Car::UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide)
{
}

void Car::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
}

glm::vec3 Car::GetInteractionPoint() const
{
    return glm::vec3();
}
