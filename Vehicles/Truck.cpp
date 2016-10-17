#include "Truck.h"

bool Truck::LoadVehicleComponents(ModelManager* modelManager)
{
    return false;
}

void Truck::Update(float gameTime, float frameTime)
{
}

void Truck::UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide)
{
}

void Truck::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
}

glm::vec3 Truck::GetInteractionPoint() const
{
    return glm::vec3();
}
