#pragma once
#include <string>
#include <map>
#include <GL/glew.h>
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

struct TerrainTile
{
    int x, y;
    GLuint heightmapTextureId;
    unsigned char* rawImage;
    float* heightmap;
};

// Defines loading and displaying a single unit of terrain.
class Terrain
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
    Terrain(ShaderManager* shaderManager, std::string terrainRootFolder, int maxTileSize, int tileSize);
    
    int GetTileSize() const;

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadTerrainTile(int x, int y, TerrainTile** tile);
    
    // Renders a tile. *The tile must have been loaded ahead-of-time.
    void RenderTile(int x, int y, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

    virtual ~Terrain();
};

