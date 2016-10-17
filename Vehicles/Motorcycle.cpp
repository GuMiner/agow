#include "Motorcycle.h"

bool Motorcycle::LoadVehicleComponents(ModelManager* modelManager)
{
    return false;
}

void Motorcycle::Update(float gameTime, float frameTime)
{
}

void Motorcycle::UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide)
{
}

void Motorcycle::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
}

glm::vec3 Motorcycle::GetInteractionPoint() const
{
    return glm::vec3();
}
