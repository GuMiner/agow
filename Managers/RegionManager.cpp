#include "Utils\Logger.h"
#include "RegionManager.h"

RegionManager::RegionManager(ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* physics, std::string terrainRootFolder, int tileSize, vec::vec2i min, vec::vec2i max, int tileViewDistance)
    : terrainManager(min, max, shaderManager, modelManager, physics, terrainRootFolder, tileSize),
      loadedRegions(), visibleTiles(), tileViewDistance(tileViewDistance)
{
    this->min = min * TerrainManager::Subdivisions;
    this->max = max * TerrainManager::Subdivisions;
    lastCenterTile = vec::vec2i(min);
}

bool RegionManager::InitializeGraphics()
{
    return terrainManager.LoadBasics();
}

float RegionManager::GetPointHeight(btDynamicsWorld* dynamicsWorld, const vec::vec2 point)
{
    vec::vec2i region = vec::vec2i((int)point.x, (int)point.y) / terrainManager.GetTileSize();
    if (region.x * TerrainManager::Subdivisions < min.x || region.x * TerrainManager::Subdivisions > max.x || region.y * TerrainManager::Subdivisions < min.y || region.y * TerrainManager::Subdivisions > max.y)
    {
        Logger::LogWarn("Attempted to get the height of a point outside the subtile boundaries: [", region.x * TerrainManager::Subdivisions, ", ", region.y * TerrainManager::Subdivisions, "].");
        return 0;
    }

    // Load the region if it hasn't been loaded already.
    if (loadedRegions.find(region) == loadedRegions.end())
    {
        loadedRegions[region] = new Region(region, &terrainManager, TerrainManager::Subdivisions);
    }

    std::vector<vec::vec2i> tileMap;
    tileMap.push_back(vec::vec2i((int)point.x, (int)point.y) / (terrainManager.GetTileSize() / TerrainManager::Subdivisions));
    loadedRegions[region]->EnsureHeightmapsLoaded(dynamicsWorld, &tileMap);
    return loadedRegions[region]->GetPointHeight(tileMap[0], vec::vec2i((int)point.x, (int)point.y));
}

int RegionManager::GetPointTerrainType(btDynamicsWorld* dynamicsWorld, const vec::vec2 point)
{
    vec::vec2i region = vec::vec2i((int)point.x, (int)point.y) / terrainManager.GetTileSize();
    if (region.x * TerrainManager::Subdivisions < min.x || region.x * TerrainManager::Subdivisions > max.x || region.y * TerrainManager::Subdivisions < min.y || region.y * TerrainManager::Subdivisions > max.y)
    {
        Logger::LogWarn("Attempted to get the terrain type of a point outside the subtile boundaries: [", region.x * TerrainManager::Subdivisions, ", ", region.y * TerrainManager::Subdivisions, "].");
        return TerrainTypes::LAKE;
    }

    // Load the region if it hasn't been loaded already.
    if (loadedRegions.find(region) == loadedRegions.end())
    {
        loadedRegions[region] = new Region(region, &terrainManager, TerrainManager::Subdivisions);
    }

    vec::vec2i regionPos = vec::vec2i((int)point.x, (int)point.y) / (terrainManager.GetTileSize() / TerrainManager::Subdivisions);
    return loadedRegions[region]->GetPointType(regionPos, vec::vec2i((int)point.x, (int)point.y));
}

vec::vec2i RegionManager::GetCurrentCenterTile(const vec::vec3& position) const
{
    // Round down.
    int scaleDownFactor = terrainManager.GetTileSize() / TerrainManager::Subdivisions;
    return vec::vec2i((int)position.x, (int)position.y) / scaleDownFactor;
}

void RegionManager::ComputeVisibleTiles(vec::vec2i centerTile, std::vector<vec::vec2i>* visibleTiles) const
{
    // Visible tiles are defined as those within the *radius* of the center tile, given the view distance.
    int minX = std::max(min.x, centerTile.x - tileViewDistance / 2);
    int minY = std::max(min.y, centerTile.y - tileViewDistance / 2);
    int maxX = std::min(max.x, centerTile.x + tileViewDistance / 2);
    int maxY = std::min(max.y, centerTile.y + tileViewDistance / 2);
    for (int i = minX; i < maxX; i++)
    {
        for (int j = minY; j < maxY; j++)
        {
            if (std::pow(i - centerTile.x, 2) + std::pow(j - centerTile.y, 2) < std::pow(tileViewDistance / 2, 2))
            {
                visibleTiles->push_back(vec::vec2i(i, j));
            }
        }
    }
}

void RegionManager::UpdateVisibleRegion(const vec::vec3& playerPosition, btDynamicsWorld* dynamicsWorld)
{
    // Update what's visible, skipping if we haven't changed center tiles.
    vec::vec2i centerTile = GetCurrentCenterTile(playerPosition);
    if (centerTile.x == lastCenterTile.x && centerTile.y == lastCenterTile.y)
    {
        return;
    }

    lastCenterTile = centerTile;

    visibleTiles.clear();
    ComputeVisibleTiles(centerTile, &visibleTiles);

    // Load regions we have not loaded yet.
    std::set<vec::vec2i, vec::vec2iComparer> visibleRegions;
    for (const vec::vec2i& visibleTile : visibleTiles)
    {
        visibleRegions.insert(visibleTile / TerrainManager::Subdivisions);
    }

    Logger::Log("Center (", centerTile.x, ", ", centerTile.y, "): visible tile results: found a total of ", visibleTiles.size(), " tiles in ", visibleRegions.size(), " regions.");
    for (const vec::vec2i visibleRegion : visibleRegions)
    {
        if (loadedRegions.find(visibleRegion) == loadedRegions.end())
        {
            loadedRegions[visibleRegion] = new Region(visibleRegion, &terrainManager, TerrainManager::Subdivisions);
        }

        loadedRegions[visibleRegion]->EnsureHeightmapsLoaded(dynamicsWorld, &visibleTiles);
    }

    // Remove regions we no longer need to keep in memory as they're very resource intensive objects.
    std::vector<vec::vec2i> regionsToRemove;
    for (std::pair<const vec::vec2i, Region*>& region : loadedRegions)
    {
        if (visibleRegions.find(region.first) == visibleRegions.end())
        {
            region.second->CleanupRegion(&terrainManager, dynamicsWorld);
            delete region.second;
            regionsToRemove.push_back(region.first);
        }
    }

    for (const vec::vec2i cleanedRegion : regionsToRemove)
    {
        loadedRegions.erase(cleanedRegion);
    }
}

void RegionManager::SimulateVisibleRegions(float gameTime, float elapsedSeconds)
{
    terrainManager.Update(gameTime);

    for (const vec::vec2i& visibleTile : visibleTiles)
    {
        vec::vec2i region = visibleTile / TerrainManager::Subdivisions;
        loadedRegions[region]->Simulate(&terrainManager, visibleTile, elapsedSeconds);
    }
}

void RegionManager::RenderRegions(const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix)
{
    for (const vec::vec2i& visibleTile : visibleTiles)
    {
        vec::vec2i region = visibleTile / TerrainManager::Subdivisions;
        loadedRegions[region]->RenderRegion(visibleTile, &terrainManager, perspectiveMatrix, viewMatrix);
    }
}

void RegionManager::CleanupPhysics(btDynamicsWorld* dynamicsWorld)
{
    for (std::pair<const vec::vec2i, Region*>& region : loadedRegions)
    {
        region.second->CleanupRegion(&terrainManager, dynamicsWorld);
        delete region.second;
    }
}

RegionManager::~RegionManager()
{

}
