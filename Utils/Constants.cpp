#include <glm/gtc/matrix_transform.hpp>
#include "Constants.h"

float Constants::FOV_Y;
float Constants::ASPECT;
float Constants::NEAR_PLANE;
float Constants::FAR_PLANE;
glm::mat4 Constants::PerspectiveMatrix;

float Constants::PI;

Constants::Constants()
{
    FOV_Y = 50.0f;
    ASPECT = 1.77778f; // 16:9
    NEAR_PLANE = 0.10f;
    FAR_PLANE = 1000.0f;
    
    PerspectiveMatrix = glm::perspective(glm::radians(Constants::FOV_Y), Constants::ASPECT, Constants::NEAR_PLANE, Constants::FAR_PLANE);

    PI = 3.141592653589f;
}
