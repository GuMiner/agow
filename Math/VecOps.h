#pragma once
#include <Bullet\LinearMath\btQuaternion.h>
#include <Bullet\LinearMath\btVector3.h>
#include "Vec.h"

// Performs vector-specific mathematical operations.
class VecOps
{
    public:
        // Quaternion conversion from bullet physics.
        static vec::quaternion VecOps::Convert(const btQuaternion& quaternion);

        // Vector conversion from bullet physics.
        static vec::vec3 Convert(const btVector3& bulletPhysicsVector);

        // Cross-product.
        static vec::vec3 Cross(const vec::vec3& first, const vec::vec3& second);

        // Dot-product.
        static float Dot(const vec::vec3& first, const vec::vec3& second);

        // Distance between two vectors
        static float Distance(const vec::vec3& first, const vec::vec3& second);

        // Angle between two vectors.
        static float Angle(const vec::vec3& first, const vec::vec3& second);
};
