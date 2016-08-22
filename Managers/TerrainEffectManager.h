#pragma once
#include <string>
#include <set>
#include <map>
#include <GL/glew.h>
#include "Data\TerrainTile.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"
#include "Utils\Vertex.h"

struct GrassEffect
{
	GLuint vao;
	GLuint positionBuffer;
	GLuint colorBuffer;

	universalVertices grassStalks;
	std::vector<vec::vec3> grassOffsets;
};

struct RoadEffect
{
	SubTile* tile;

	GLuint vao;
	GLuint positionBuffer;
	GLuint colorBuffer;

	universalVertices travellers;
	std::vector<vec::vec2i> roadPositions;
};

struct EffectData
{
	bool hasGrassEffect;
	GrassEffect grassEffect;

	bool hasRoadEffect;
	RoadEffect roadEffect;
};

struct GrassProgram
{
	GLuint programId;

	GLuint projMatrixLocation;
	GLuint mvMatrixLocation;
};

struct RoadProgram
{
	GLuint programId;

	GLuint projMatrixLocation;
	GLuint mvMatrixLocation;
};

// Defines effects for each sub tile of terrain.
class TerrainEffectManager
{
    ShaderManager* shaderManager;
    int subTileSize; // In pixels

	GrassProgram grassProgram;
	RoadProgram roadProgram;

    std::map<vec::vec2i, EffectData*, vec::vec2iComparer> effectData;
	
	bool GetNearbyCheckOne(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
	bool GetNearbyCheckTwo(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
	bool GetNearbyCheckThree(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
	bool GetNearbyCheckFour(SubTile* tile, const vec::vec2i pos, const int terrainType, vec::vec2i* result) const;
	vec::vec2i GetNearbyType(SubTile* tile, const vec::vec2i pos, const int terrainType) const;

	void LoadGrassEffect(vec::vec2i pos, EffectData* effect, SubTile* tile);
	void UnloadGrassEffect(vec::vec2i pos);

	void LoadRoadEffect(vec::vec2i pos, EffectData* effect, SubTile* tile);
	void UnloadRoadEffect(vec::vec2i pos);

	void CleanupSubTileEffects(vec::vec2i pos, bool log);

public:
	TerrainEffectManager(ShaderManager* shaderManager, int subTileSize);

    // Loads generic OpenGL functionality needed.
    bool LoadBasics();
    
    // Loads a single tile.
    bool LoadSubTileEffects(vec::vec2i pos, SubTile* tile);
    
	// Simulates the effects for the loaded tile.
	void Simulate(const vec::vec2i pos, float elapsedSeconds);

    // Renders a tile's effects *The tile must have been loaded ahead-of-time.*
    void RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

	void UnloadSubTileEffects(vec::vec2i pos);
    virtual ~TerrainEffectManager();
};

