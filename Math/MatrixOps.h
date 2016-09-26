#pragma once
#include <deque>
#include "Vec.h"

// Performs matrix-specific math operations.
class MatrixOps
{
    public:
        // Computes a perspective matrix with the provided FOV-Y, aspect, near plane, and far plane.
        static vec::mat4 Perspective(float fovy, float aspect, float n, float f);

        // Computes a look-at matrix
        static vec::mat4 Lookat(const vec::vec3& eye, const vec::vec3& center, const vec::vec3& up);

        // Computes a translation matrix.
        static vec::mat4 Translate(float x, float y, float z);
        static vec::mat4 Translate(const vec::vec3& v);

        // Computes a scaling matrix.
        static vec::mat4 Scale(float x, float y, float z);
        static vec::mat4 Scale(const vec::vec3& v);

        //Computes an xy shear matrix, leaving the z-axis the same. If x and y are zero, this is equal to the identity matrix.
        static vec::mat4 Shear(float x, float y);

        // Computes an axis-angle rotation matrix.
        static vec::mat4 Rotate(float angle, float x, float y, float z);
        static vec::mat4 Rotate(float angle, const vec::vec3& v);

        // Computes the inverse of the provided matrix, filling it into the result.
        static void Inverse(const vec::mat4& matrix, vec::mat4& result);

        // Averages two matrices togehter.
        static vec::mat4 Average(vec::mat4 first, vec::mat4 second);
        static vec::mat4 Average(std::deque<vec::mat4> matrices);
};
