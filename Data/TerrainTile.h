#pragma once
#include <GL/glew.h>

// Holds the graphics and physics raw information about an in-game tile.
struct TerrainTile
{
    int x, y;
    GLuint heightmapTextureId;
    unsigned char* rawImage;
    float* heightmap;
};