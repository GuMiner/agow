#pragma once
#include <string>
#include <set>
#include <map>
#include <GL/glew.h>
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Managers\TerrainEffectManager.h"
#include <glm\vec3.hpp>
#include "BasicPhysics.h"

// Defines loading and displaying a single unit of terrain.
class TerrainManager
{
    glm::ivec2 min;
    glm::ivec2 max;

    ShaderManager* shaderManager;
    std::string rootFolder;
    int tileSize; // In pixels

    GLuint terrainRenderProgram;

    GLuint terrainTexLocation;
    GLuint terrainTypeTexLocation;
    GLuint gameTimeLocation;
    GLuint mvLocation;
    GLuint projLocation;

    float lastGameTime;

    TerrainEffectManager terrainEffects;
    std::map<glm::ivec2, TerrainTile*, iVec2Comparer> terrainTiles;

    // Given a terrain tile, creates an appropriate heightmap texture for it.
    GLuint CreateTileTexture(GLenum activeTexture, int subSize, float* heightmap);
    GLuint CreateTileTexture(GLenum activeTexture, int subSize, unsigned char* heightmap);
    bool LoadTileToCache(glm::ivec2 start, bool loadSubtiles);
    void LoadHeightmapEdges(const glm::ivec2& start, int i, int j, int subSize, float* heightmap);

    void CleanupTerrainTile(glm::ivec2 start, bool log);

public:
    TerrainManager(glm::ivec2 min, glm::ivec2 max, ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* physics, std::string terrainRootFolder, int tileSize);
    
    static const int Subdivisions = 10;
    int GetTileSize() const;
    int GetSubTileSize() const;

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadTerrainTile(glm::ivec2 start, TerrainTile** tile);
    
    // Runs simulations on a loaded tile.
    void Update(float gameTime);
    void Simulate(const glm::ivec2 start, const glm::ivec2 subPos, float elapsedSeconds);

    // Renders a tile. *The tile must have been loaded ahead-of-time.*
    void RenderTile(const glm::ivec2 start, const glm::ivec2 subPos, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix);

    void UnloadTerrainTile(glm::ivec2 start);
    virtual ~TerrainManager();
};

