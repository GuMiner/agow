#pragma once
#include <string>
#include <map>
#include <GL/glew.h>
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

// Defines loading and displaying a single unit of terrain.
class TerrainManager
{
    ShaderManager* shaderManager;
    std::string rootFolder;
    int maxTileSideCount; // Total number of tiles on each side of the total map.
    int tileSize; // In pixels

    GLuint terrainRenderProgram;

    GLuint terrainTexLocation;
    GLuint mvLocation;
    GLuint projLocation;

    std::map<int, TerrainTile*> terrainTiles;

    int GetTileId(int x, int y) const;

    // Given a terrain tile, creates an appropriate heightmap texture for it.
    GLuint CreateHeightmapTexture(TerrainTile* tile);

public:
    TerrainManager(ShaderManager* shaderManager, std::string terrainRootFolder, int maxTileSize, int tileSize);
    
	static const int Subdivisions = 10;
    int GetTileSize() const;

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadTerrainTile(int x, int y, TerrainTile** tile);
    
    // Renders a tile. *The tile must have been loaded ahead-of-time.*
    void RenderTile(int x, int y, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

    virtual ~TerrainManager();
};

