#include "Tank.h"

bool Tank::LoadVehicleComponents(ModelManager* modelManager)
{
    return false;
}

void Tank::Update(float gameTime, float frameTime)
{
}

void Tank::UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide)
{
}

void Tank::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
}

glm::vec3 Tank::GetInteractionPoint() const
{
    return glm::vec3();
}
