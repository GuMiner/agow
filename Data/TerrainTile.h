#pragma once
#include <map>
#include <GL/glew.h>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>

// TODO make a new 'math utils' class and put this there.
struct iVec2Comparer
{
    bool operator()(const glm::ivec2& lhs, const glm::ivec2& rhs) const
    {
        // TODO this is really wrong (assuming max voxel size), but also right (anything that big wouldn't run.
        const int maxSize = 10000;
        return (lhs.x + lhs.y * maxSize) > (rhs.x + rhs.y * maxSize);
    }
};

// Copied from PaletteWindow.cpp, MapEditor.
namespace TerrainTypes
{
    const int SNOW_PEAK = 0;
    const int ROCKS = 25;
    const int TREES = 50;
    const int DIRTLAND = 75;
    const int GRASSLAND = 100;
    const int ROADS = 125;
    const int CITY = 150;
    const int SAND = 175;
    const int RIVER = 200;
    const int LAKE = 225;
}

struct SubTile
{
    GLuint heightmapTextureId;
    float* heightmap;

    GLuint typeTextureId;
    unsigned char* type;

    SubTile(GLuint heightmapTextureId, float* heightmap, GLuint typeTextureId, unsigned char* type)
        : heightmapTextureId(heightmapTextureId), heightmap(heightmap), typeTextureId(typeTextureId), type(type)
    {
    }
};

// Holds the graphics and physics raw information about an in-game tile.
struct TerrainTile
{
    unsigned char* rawImage;
    bool loadedSubtiles;

    std::map<glm::ivec2, SubTile*, iVec2Comparer> subtiles;
};