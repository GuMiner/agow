#pragma once
#include <string>
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
    std::vector<Region> loadedRegions;

    // Extents we can actually transverse in the tiles.
    vec::vec2i min;
    vec::vec2i max;

public:
    RegionManager(ShaderManager* shaderManager, std::string terrainRootFolder, 
        int maxTileCount, int tileSize, vec::vec2i min, vec::vec2i max);
    bool InitializeGraphics();
    
    void UpdateVisibleRegion(const vec::vec3& playerPosition, btDynamicsWorld* dynamicsWorld);
    void RenderRegions(const vec::mat4& projectionMatrix);

    void CleanupPhysics(btDynamicsWorld* dynamicsWorld);
    virtual ~RegionManager();
};

