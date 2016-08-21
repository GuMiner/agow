#pragma once
#include <string>
#include <set>
#include <map>
#include <GL/glew.h>
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

struct EffectData
{

};

// Defines effects for each sub tile of terrain.
class TerrainEffectManager
{
    ShaderManager* shaderManager;
    int subTileSize; // In pixels

    //GLuint terrainRenderProgram;

    //GLuint terrainTexLocation;
	//GLuint terrainTypeTexLocation;
    //GLuint mvLocation;
    //GLuint projLocation;

    std::map<vec::vec2i, EffectData*, vec::vec2iComparer> effectData;
	
	void CleanupSubTileEffects(vec::vec2i pos, bool log);

public:
	TerrainEffectManager(ShaderManager* shaderManager, int subTileSize);
    
	static const int Subdivisions = 10;
    int GetTileSize() const;
	int GetSubTileSize() const;

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadSubTileEffects(vec::vec2i pos, SubTile* tile);
    
    // Renders a tile's effects *The tile must have been loaded ahead-of-time.*
    void RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

	void UnloadSubTileEffects(vec::vec2i pos);
    virtual ~TerrainEffectManager();
};

