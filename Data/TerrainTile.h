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

// Forward declare for use in TerrainTile.
struct SubTile;

// Holds the graphics and physics raw information about an in-game tile.
struct TerrainTile
{
    static const int TileSize = 1000;
    static const int SubtileSize = 100;
    static const int Subdivisions = TileSize / SubtileSize;

    unsigned char* rawImage;
    bool loadedSubtiles;

    std::map<glm::ivec2, SubTile*, iVec2Comparer> subtiles;

    // Returns the real position of the pixel (lower X, lower Y)
    static glm::vec2 GetRealPosition(glm::ivec2 subtileId, glm::ivec2 subtilePixel)
    {
        return glm::vec2((float)subtileId.x, (float)subtileId.y) * (float)SubtileSize + glm::vec2((float)subtilePixel.x, (float)subtilePixel.y);
    }
};

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

    int GetPixelId(const glm::ivec2& pos)
    {
        return pos.x + pos.y * TerrainTile::SubtileSize;
    }
};