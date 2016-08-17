#include "Utils\Logger.h"
#include "RegionManager.h"

RegionManager::RegionManager(ShaderManager* shaderManager, std::string terrainRootFolder,
	int maxTileCount, int tileSize, vec::vec2i min, vec::vec2i max, int tileViewDistance)
	: terrainManager(shaderManager, terrainRootFolder, maxTileCount, tileSize),
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
			if (std::pow(i - centerTile.x, 2) + std::pow(j - centerTile.y, 2) < std::pow(tileViewDistance, 2))
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
	ComputeVisibleTiles(centerTile, &visibleTiles);

	// Load regions we have not loaded yet.
	std::set<vec::vec2i, vec::vec2iComparer> visibleRegions;
	for (const vec::vec2i& visibleTile : visibleTiles)
	{
		visibleRegions.insert(visibleTile / TerrainManager::Subdivisions);
	}

	Logger::Log("Center (", centerTile.x, ", ", centerTile.y, "): visible tile results: found a total of ", visibleTiles.size(), " tiles in ", visibleRegions.size(), " regions.");
	for (const vec::vec2i& visibleRegion : visibleRegions)
	{
		if (loadedRegions.find(visibleRegion) == loadedRegions.end())
		{
			loadedRegions[visibleRegion] = Region(visibleRegion, dynamicsWorld, &terrainManager, TerrainManager::Subdivisions);
		}
	}

	// TODO unload regions if we don't anticipate reentering that area to save on memory & GPU resources.
}

void RegionManager::RenderRegions(const vec::mat4& projectionMatrix)
{
	// TODO remove this code to allow region multi-rendering once rendering the selected tile only renders
	//  that tile.
	std::set<vec::vec2i, vec::vec2iComparer> renderedRegions;
    for (const vec::vec2i& visibleTile : visibleTiles)
    {
		vec::vec2i region = visibleTile / TerrainManager::Subdivisions;
		if (renderedRegions.find(region) == renderedRegions.end())
		{
			loadedRegions[region].RenderRegion(visibleTile, &terrainManager, projectionMatrix);
			renderedRegions.insert(region);
		}
    }
}

void RegionManager::CleanupPhysics(btDynamicsWorld* dynamicsWorld)
{
    for (std::pair<const vec::vec2i, Region>& region : loadedRegions)
    {
		region.second.CleanupRegion(dynamicsWorld);
    }
}

RegionManager::~RegionManager()
{

}
