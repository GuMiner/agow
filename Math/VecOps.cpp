#include <cmath>
#include "VecOps.h"

vec::quaternion VecOps::Convert(const btQuaternion& quaternion)
{
    // We need to negate this because we are in a left-hand system.
    return vec::quaternion(quaternion.getX(), quaternion.getY(), quaternion.getZ(), -quaternion.getW());
}

vec::vec3 VecOps::Convert(const btVector3& vector)
{
    return vec::vec3(vector.getX(), vector.getY(), vector.getZ());
}

// Vector conversion to bullet physics.
btVector3 VecOps::Convert(const vec::vec3& physicsVector)
{
	return btVector3::btVector3(physicsVector.x, physicsVector.y, physicsVector.z);
}

vec::vec3 VecOps::Cross(const vec::vec3& first, const vec::vec3& second)
{
    return vec::vec3(
        first.y * second.z - second.y * first.z,
        first.z * second.x - second.z * first.x,
        first.x * second.y - second.x * first.y);
}

float VecOps::Dot(const vec::vec3& first, const vec::vec3& second)
{
    vec::vec3 multiplied = first * second;
    return multiplied.x + multiplied.y + multiplied.z;
}

// Distance between two vectors
float VecOps::Distance(const vec::vec3& first, const vec::vec3& second)
{
    return vec::length(second - first);
}

// Angle between two vectors.
float VecOps::Angle(const vec::vec3& first, const vec::vec3& second)
{
    return std::acos(Dot(first, second));
}
