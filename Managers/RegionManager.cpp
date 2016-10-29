#include "Utils\Logger.h"
#include "RegionManager.h"

RegionManager::RegionManager(ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* physics, std::string terrainRootFolder, glm::ivec2 min, glm::ivec2 max, int tileViewDistance)
    : terrainManager(min, max, shaderManager, modelManager, physics, terrainRootFolder),
      loadedRegions(), visibleTiles(), tileViewDistance(tileViewDistance)
{
    this->min = min * TerrainTile::Subdivisions;
    this->max = max * TerrainTile::Subdivisions;
    lastCenterTile = glm::ivec2(min);
}

bool RegionManager::InitializeGraphics()
{
    return terrainManager.LoadBasics();
}

float RegionManager::GetPointHeight(btDynamicsWorld* dynamicsWorld, const glm::vec2 point)
{
    glm::ivec2 region = glm::ivec2((int)point.x, (int)point.y) / TerrainTile::TileSize;
    glm::ivec2 subtileRegion = glm::ivec2((int)point.x, (int)point.y) / TerrainTile::SubtileSize;
    if (subtileRegion.x < min.x || subtileRegion.x > max.x || subtileRegion.y < min.y || subtileRegion.y > max.y)
    {
        Logger::LogWarn("Attempted to get the height of a point outside the subtile boundaries: [", region.x, ", ", region.y, "].");
        return 0;
    }

    // Load the region if it hasn't been loaded already.
    if (loadedRegions.find(region) == loadedRegions.end())
    {
        loadedRegions[region] = new Region(region, &terrainManager);
    }

    std::vector<glm::ivec2> tileMap;
    tileMap.push_back(subtileRegion);
    loadedRegions[region]->EnsureHeightmapsLoaded(dynamicsWorld, &tileMap);
    return loadedRegions[region]->GetPointHeight(tileMap[0], glm::ivec2((int)point.x, (int)point.y));
}

int RegionManager::GetPointTerrainType(btDynamicsWorld* dynamicsWorld, const glm::vec2 point)
{
    glm::ivec2 region = glm::ivec2((int)point.x, (int)point.y) / TerrainTile::TileSize;
    glm::ivec2 subtileRegion = glm::ivec2((int)point.x, (int)point.y) / TerrainTile::SubtileSize;
    if (subtileRegion.x < min.x || subtileRegion.x > max.x || subtileRegion.y < min.y || subtileRegion.y  > max.y)
    {
        Logger::LogWarn("Attempted to get the terrain type of a point outside the subtile boundaries: [", region.x, ", ", region.y, "].");
        return TerrainTypes::LAKE;
    }

    // Load the region if it hasn't been loaded already.
    if (loadedRegions.find(region) == loadedRegions.end())
    {
        loadedRegions[region] = new Region(region, &terrainManager);
    }

    return loadedRegions[region]->GetPointType(subtileRegion, glm::ivec2((int)point.x, (int)point.y));
}

glm::ivec2 RegionManager::GetCurrentCenterTile(const glm::vec3& position) const
{
    // Round down.
    return glm::ivec2((int)position.x, (int)position.y) / TerrainTile::SubtileSize;
}

void RegionManager::ComputeVisibleTiles(glm::ivec2 centerTile, std::vector<glm::ivec2>* visibleTiles) const
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
                visibleTiles->push_back(glm::ivec2(i, j));
            }
        }
    }
}

void RegionManager::UpdateVisibleRegion(const glm::vec3& playerPosition, btDynamicsWorld* dynamicsWorld)
{
    // Update what's visible, skipping if we haven't changed center tiles.
    glm::ivec2 centerTile = GetCurrentCenterTile(playerPosition);
    if (centerTile.x == lastCenterTile.x && centerTile.y == lastCenterTile.y)
    {
        return;
    }

    lastCenterTile = centerTile;

    visibleTiles.clear();
    ComputeVisibleTiles(centerTile, &visibleTiles);

    // Load regions we have not loaded yet.
    std::set<glm::ivec2, iVec2Comparer> visibleRegions;
    for (const glm::ivec2& visibleTile : visibleTiles)
    {
        visibleRegions.insert(visibleTile / TerrainTile::Subdivisions);
    }

    Logger::Log("Center (", centerTile.x, ", ", centerTile.y, "): visible tile results: found a total of ", visibleTiles.size(), " tiles in ", visibleRegions.size(), " regions.");
    for (const glm::ivec2 visibleRegion : visibleRegions)
    {
        if (loadedRegions.find(visibleRegion) == loadedRegions.end())
        {
            loadedRegions[visibleRegion] = new Region(visibleRegion, &terrainManager);
        }

        loadedRegions[visibleRegion]->EnsureHeightmapsLoaded(dynamicsWorld, &visibleTiles);
    }

    // Remove regions we no longer need to keep in memory as they're very resource intensive objects.
    std::vector<glm::ivec2> regionsToRemove;
    for (std::pair<const glm::ivec2, Region*>& region : loadedRegions)
    {
        if (visibleRegions.find(region.first) == visibleRegions.end())
        {
            region.second->CleanupRegion(&terrainManager, dynamicsWorld);
            delete region.second;
            regionsToRemove.push_back(region.first);
        }
    }

    for (const glm::ivec2 cleanedRegion : regionsToRemove)
    {
        loadedRegions.erase(cleanedRegion);
    }
}

void RegionManager::SimulateVisibleRegions(float gameTime, float elapsedSeconds)
{
    terrainManager.Update(gameTime);

    for (const glm::ivec2& visibleTile : visibleTiles)
    {
        glm::ivec2 region = visibleTile / TerrainTile::Subdivisions;
        loadedRegions[region]->Simulate(&terrainManager, visibleTile, elapsedSeconds);
    }
}

void RegionManager::RenderRegions(const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix)
{
    for (const glm::ivec2& visibleTile : visibleTiles)
    {
        glm::ivec2 region = visibleTile / TerrainTile::Subdivisions;
        loadedRegions[region]->RenderRegion(visibleTile, &terrainManager, perspectiveMatrix, viewMatrix);
    }
}

void RegionManager::CleanupPhysics(btDynamicsWorld* dynamicsWorld)
{
    for (std::pair<const glm::ivec2, Region*>& region : loadedRegions)
    {
        region.second->CleanupRegion(&terrainManager, dynamicsWorld);
        delete region.second;
    }
}

RegionManager::~RegionManager()
{

}
