#include "PhysicsOps.h"

glm::vec3 PhysicsOps::XY_PLANE_NORMAL;
glm::vec3 PhysicsOps::YZ_PLANE_NORMAL;
glm::vec3 PhysicsOps::XZ_PLANE_NORMAL;

// Determines if a given ray hits a given plane. Returns true and fills in the intersection factor ('t', where rs + t*ray = intersection point).
bool PhysicsOps::HitsPlane(const glm::vec3& rayStart, const glm::vec3& ray, const glm::vec3& planeNormal, const glm::vec3& planePoint, float* intersectionFactor)
{
    float rayDotNormal = glm::dot(ray, planeNormal);
    if (rayDotNormal == 0)
    {
        return false;
    }

    *intersectionFactor = glm::dot(planePoint - rayStart, planeNormal) / rayDotNormal;
    return true;
}

bool PhysicsOps::HitsPlane(const glm::vec3& rayStart, const glm::vec3& ray, Plane plane, const glm::vec3& planePoint, float* intersectionFactor)
{
    glm::vec3 actualPlane;
    switch (plane)
    {
    case Plane::XY:
        actualPlane = XY_PLANE_NORMAL;
        break;
    case Plane::YZ:
        actualPlane = YZ_PLANE_NORMAL;
        break;
    case Plane::XZ:
    default:
        actualPlane = XZ_PLANE_NORMAL;
        break;
    }

    return HitsPlane(rayStart, ray, actualPlane, planePoint, intersectionFactor);
}

bool PhysicsOps::HitsSphere(const glm::vec3& rayStart, const glm::vec3& ray, const glm::vec3& sphereCenter, float sphereRadius)
{
    // After some work on paper, it ends up that you just need to calculate a determinant and ensure it is > 0
    glm::vec3 rayOffset = rayStart - sphereCenter;

    float first = pow(glm::dot(ray, rayOffset), 2);
    float second = glm::dot(rayOffset, rayOffset) - pow(sphereRadius, 2);

    return first >= second; // first - second >= 0
}

// Computes a ray from the current mouse position into the scene.
glm::vec3 PhysicsOps::ScreenRay(glm::vec2 mouse, glm::vec2 screenSize, glm::mat4& perspectiveMatrix, glm::mat4& viewRotationMatrix)
{
    // Scale from -1.0 to 1.0, and invert Y
    glm::vec2 deviceCoords = (mouse * 2.0f - screenSize) / screenSize;
    deviceCoords.y = -deviceCoords.y;

    // Point the ray away from us.
    glm::vec4 clippedRay = glm::vec4(deviceCoords.x, deviceCoords.y, -1.0f, 1.0f);

    // Invert our projection and use the normal view matrix to end up with a world ray, which is exactly what we want.
    glm::mat4 invPerspectiveMatrix = glm::inverse(perspectiveMatrix);
    glm::vec4 eyeRay = clippedRay * invPerspectiveMatrix;
    eyeRay[2] = -1.0f;
    eyeRay[3] = 0.0f;

    glm::vec4 worldRay = eyeRay * viewRotationMatrix;

    glm::vec3 result = glm::vec3(worldRay[0], worldRay[1], worldRay[2]);
    return normalize(result);
}

glm::mat4 PhysicsOps::Shear(float xAmount, float yAmount)
{
    return glm::mat4(
        glm::vec4(1.0f, 0.0f, xAmount, 0.0f),
        glm::vec4(0.0f, 1.0f, yAmount, 0.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

// Converts a GLM vector to a Bullet Physics vector.
btVector3 PhysicsOps::Convert(glm::vec3 vector)
{
    return btVector3(vector.x, vector.y, vector.z);
}

glm::vec3 PhysicsOps::ForwardsVector(glm::quat quaternion)
{
    glm::vec3 DEFAULT_FORWARD_VECTOR(0.0f, 0.0f, 1.0f);
    return quaternion * DEFAULT_FORWARD_VECTOR;
}

glm::vec3 PhysicsOps::UpVector(glm::quat quaternion)
{
    glm::vec3 DEFAULT_UP_VECTOR(0.0f, 1.0f, 0.0f);
    return quaternion * DEFAULT_UP_VECTOR;
}

glm::mat4 PhysicsOps::Average(std::deque<glm::mat4> matrices)
{
    glm::mat4 result(glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f));
    for (auto iter = matrices.cbegin(); iter != matrices.cend(); iter++)
    {
        result[0] += (*iter)[0];
        result[1] += (*iter)[1];
        result[2] += (*iter)[2];
        result[3] += (*iter)[3];
    }
    
    result[0] /= (float)matrices.size();
    result[1] /= (float)matrices.size();
    result[2] /= (float)matrices.size();
    result[3] /= (float)matrices.size();

    return result;
}