#pragma once
#include "Math\MatrixOps.h"

// Holds data constant across the application.
class Constants
{
public:
    Constants();

    // Allowable return codes
    enum Status { OK = 0, BAD_SHADERS = 1, BAD_FONT = 2,
        BAD_IMAGES = 3, BAD_CONFIG = 4, BAD_GLEW = 5, BAD_STATS = 6, BAD_SCENERY = 7,
        BAD_PHYSICS = 8, BAD_TERRAIN = 9, BAD_MODEL = 10, BAD_DIALOG = 11 };

    // Graphics viewport settings
    static float FOV_Y;
    static float ASPECT;
    static float NEAR_PLANE;
    static float FAR_PLANE;

    static float PI;

    static vec::mat4 PerspectiveMatrix;

    const static int MAX_FRAMERATE = 60;
};

