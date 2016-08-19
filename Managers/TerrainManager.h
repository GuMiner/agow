#pragma once
#include <string>
#include <set>
#include <map>
#include <GL/glew.h>
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

// Defines loading and displaying a single unit of terrain.
class TerrainManager
{
	vec::vec2i min;
	vec::vec2i max;

    ShaderManager* shaderManager;
    std::string rootFolder;
    int tileSize; // In pixels

    GLuint terrainRenderProgram;

    GLuint terrainTexLocation;
	GLuint terrainTypeTexLocation;
    GLuint mvLocation;
    GLuint projLocation;

    std::map<vec::vec2i, TerrainTile*, vec::vec2iComparer> terrainTiles;

    // Given a terrain tile, creates an appropriate heightmap texture for it.
    GLuint CreateTileTexture(GLenum activeTexture, int subSize, float* heightmap);
	GLuint CreateTileTexture(GLenum activeTexture, int subSize, unsigned char* heightmap);
	bool LoadTileToCache(vec::vec2i pos, bool loadSubtiles);

public:
    TerrainManager(vec::vec2i min, vec::vec2i max, ShaderManager* shaderManager, std::string terrainRootFolder, int tileSize);
    
	static const int Subdivisions = 10;
    int GetTileSize() const;
	int GetSubTileSize() const;

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadTerrainTile(vec::vec2i pos, TerrainTile** tile);
    
    // Renders a tile. *The tile must have been loaded ahead-of-time.*
    void RenderTile(const vec::vec2i pos, const vec::vec2i subPos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

	void CleanupTerrainTile(vec::vec2i pos, bool log);
	void UnloadTerrainTile(vec::vec2i pos);

    virtual ~TerrainManager();
};

