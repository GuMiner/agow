#pragma once
#include <map>
#include <GL/glew.h>
#include "Math\Vec.h"

struct SubTile
{
	GLuint heightmapTextureId;
	float* heightmap;

	SubTile(GLuint heightmapTextureId, float* heightmap)
		: heightmapTextureId(heightmapTextureId), heightmap(heightmap)
	{
	}
};

// Holds the graphics and physics raw information about an in-game tile.
struct TerrainTile
{
    unsigned char* rawImage;
	bool loadedSubtiles;

	std::map<vec::vec2i, SubTile*, vec::vec2iComparer> subtiles;
};