#include "Utils\Logger.h"
#include "RegionManager.h"

RegionManager::RegionManager(ShaderManager* shaderManager, std::string terrainRootFolder,
    int maxTileCount, int tileSize, vec::vec2i min, vec::vec2i max)
    : terrainManager(shaderManager, terrainRootFolder, maxTileCount, tileSize),
      min(min), max(max)
{

}

bool RegionManager::InitializeGraphics()
{
    return terrainManager.LoadBasics();
}

void RegionManager::UpdateVisibleRegion(const vec::vec3& playerPosition, btDynamicsWorld* dynamicsWorld)
{
    if (loadedRegions.size() == 0)
    {
        loadedRegions.push_back(Region(21, 10, dynamicsWorld, &terrainManager, 10, 10));
        loadedRegions.push_back(Region(22, 10, dynamicsWorld, &terrainManager, 10, 10));
        //loadedRegions.push_back(Region(22, 11, dynamicsWorld, &terrainManager, 10, 10));
        //loadedRegions.push_back(Region(21, 11, dynamicsWorld, &terrainManager, 10, 10));
    }

    // terrain.LoadTerrainTile(21, 10, &testTile);
}

void RegionManager::RenderRegions(const vec::mat4& projectionMatrix)
{
    for (const Region& region : loadedRegions)
    {
        region.RenderRegion(&terrainManager, projectionMatrix);
    }
}

void RegionManager::CleanupPhysics(btDynamicsWorld* dynamicsWorld)
{
    for (Region& region : loadedRegions)
    {
        region.CleanupRegion(dynamicsWorld);
    }
}

RegionManager::~RegionManager()
{

}
