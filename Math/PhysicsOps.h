#pragma once
#include <deque>
#include <Bullet\btBulletDynamicsCommon.h>
#include <glm\gtc\quaternion.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>

// Holds physics-specific mathematical operations
class PhysicsOps
{
    public:
        enum Plane
        {
            XY = 0,
            YZ,
            XZ
        };

        static glm::vec3 XY_PLANE_NORMAL;
        static glm::vec3 YZ_PLANE_NORMAL;
        static glm::vec3 XZ_PLANE_NORMAL;

        PhysicsOps()
        {
            XY_PLANE_NORMAL = glm::vec3(0.0f, 0.0f, 1.0f);
            YZ_PLANE_NORMAL = glm::vec3(1.0f, 0.0f, 0.0f);
            XZ_PLANE_NORMAL = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // Determines if a given ray hits a given plane. Returns true and fills in the intersection factor ('t', where rs + t*ray = intersection point).
        static bool HitsPlane(const glm::vec3& rayStart, const glm::vec3& ray, const glm::vec3& planeNormal, const glm::vec3& planePoint, float* intersectionFactor);
        static bool HitsPlane(const glm::vec3& rayStart, const glm::vec3& ray, Plane plane, const glm::vec3& planePoint, float* intersectionFactor);

        // Determines if a given ray hits a sphere. Returns true if so, false otherwise.
        static bool HitsSphere(const glm::vec3& rayStart, const glm::vec3& ray, const glm::vec3& sphereCenter, float sphereRadius);

        // Computes a ray from the current mouse position into the scene.
        static glm::vec3 ScreenRay(glm::vec2 mouse, glm::vec2 screenSize, glm::mat4& perspectiveMatrix, glm::mat4& viewRotationMatrix);

        // Creates a shear matrix
        static glm::mat4 Shear(float xAmount, float yAmount);

        // Converts to and from bullet physics vector types.
        static btVector3 Convert(glm::vec3 vector);

        // The 'up' and 'forwards' vector given the current quaternion rotation applied to the vectors.
        static glm::vec3 ForwardsVector(glm::quat quaternion);
        static glm::vec3 UpVector(glm::quat quaternion);

        static glm::mat4 Average(std::deque<glm::mat4> matrices);
};

