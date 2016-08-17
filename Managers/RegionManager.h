#pragma once
#include <string>
#include <set>
#include <vector>
#include <GL/glew.h>
#include <Bullet\btBulletDynamicsCommon.h>
#include "Managers\ShaderManager.h"
#include "Managers\TerrainManager.h"
#include "Math\Vec.h"
#include "Region.h"

class RegionManager
{
    TerrainManager terrainManager;
    std::map<vec::vec2i, Region, vec::vec2iComparer> loadedRegions;

	// Each region is subdivided into 100 tiles.
	std::vector<vec::vec2i> visibleTiles;

    // Extents we can actually transverse in the tiles.
    vec::vec2i min;
    vec::vec2i max;
	int tileViewDistance;

	vec::vec2i lastCenterTile;
	vec::vec2i GetCurrentCenterTile(const vec::vec3& position) const;
	void ComputeVisibleTiles(vec::vec2i centerTile, std::vector<vec::vec2i>* visibleTiles) const;

public:
    RegionManager(ShaderManager* shaderManager, std::string terrainRootFolder, 
        int maxTileCount, int tileSize, vec::vec2i min, vec::vec2i max, int tileViewDistance);
    bool InitializeGraphics();
    
    void UpdateVisibleRegion(const vec::vec3& playerPosition, btDynamicsWorld* dynamicsWorld);
    void RenderRegions(const vec::mat4& projectionMatrix);

    void CleanupPhysics(btDynamicsWorld* dynamicsWorld);
    virtual ~RegionManager();
};

