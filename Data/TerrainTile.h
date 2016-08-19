#pragma once
#include <map>
#include <GL/glew.h>
#include "Math\Vec.h"

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

	std::map<vec::vec2i, SubTile*, vec::vec2iComparer> subtiles;
};