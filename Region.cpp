#include "Region.h"
#include "Config\PhysicsConfig.h"
#include "Math\MatrixOps.h"

Region::Region(vec::vec2i pos, TerrainManager* terrainManager, int subdivisions)
    : pos(pos), subdivisions(subdivisions)
{
    terrainManager->LoadTerrainTile(pos, &regionTile);
}

vec::vec2i Region::GetPos() const
{
	return pos;
}

void Region::EnsureHeightmapsLoaded(btDynamicsWorld* dynamicsWorld, const std::vector<vec::vec2i>* tilesToLoadHeightmapsFor)
{
	for (const vec::vec2i& tilePos : *tilesToLoadHeightmapsFor)
	{
		vec::vec2i localPos = tilePos - (pos * TerrainManager::Subdivisions);
		bool inRegion = (localPos.x >= 0 && localPos.y >= 0 && localPos.x < TerrainManager::Subdivisions && localPos.y < TerrainManager::Subdivisions);
		if (inRegion && loadedHeightmaps.find(localPos) == loadedHeightmaps.end())
		{
			loadedHeightmaps[localPos] = CreateHeightmap(tilePos, regionTile->subtiles[localPos], dynamicsWorld);
		}
	}
}

btRigidBody* Region::CreateHeightmap(vec::vec2i tilePos, SubTile* subTile, btDynamicsWorld* dynamicsWorld)
{
    btHeightfieldTerrainShape* heighfield = new btHeightfieldTerrainShape(
        PhysicsConfig::TerrainSize / TerrainManager::Subdivisions, PhysicsConfig::TerrainSize / TerrainManager::Subdivisions, subTile->heightmap, 900.0f, 2, true, false);
    heighfield->setMargin(2.0f);

    // Position the heightfield so that it's not repositioned incorrectly.
    btTransform heightfieldPos;
	heightfieldPos.setIdentity();
	heightfieldPos.setOrigin(btVector3(
        (float)(tilePos.x + 0.5f) * PhysicsConfig::TerrainSize / TerrainManager::Subdivisions,
        (float)(tilePos.y + 0.5f) * PhysicsConfig::TerrainSize / TerrainManager::Subdivisions, 450.0f - 2.0f));

    btDefaultMotionState *motionState = new btDefaultMotionState(heightfieldPos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, heighfield);

    btTransform transform;
    btVector3 min, max;

    heighfield->getAabb(transform, min, max);

    btRigidBody* heightmap =  new btRigidBody(ground);
	dynamicsWorld->addRigidBody(heightmap);
	return heightmap;
}

float Region::GetPointHeight(const vec::vec2i tilePos, const vec::vec2i fullPos) const
{
    // Map to the correct subtile.
    vec::vec2i localPos = tilePos - (pos * TerrainManager::Subdivisions);

    // Map to the correct point within the subtile.
    int subTileSize = (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions);
    vec::vec2i subtileOffset = fullPos - (tilePos * subTileSize);
    return regionTile->subtiles[localPos]->heightmap[subtileOffset.x + subtileOffset.y * subTileSize];
}

void Region::Simulate(TerrainManager* terrainManager, vec::vec2i tilePos, float elapsedSeconds)
{
	terrainManager->Simulate(pos, tilePos - (pos * TerrainManager::Subdivisions), elapsedSeconds);
}

void Region::RenderRegion(vec::vec2i tilePos, TerrainManager* terrainManager, const vec::mat4& projectionMatrix) const
{
    vec::mat4 mvMatrix = MatrixOps::Translate(
        (float)(tilePos.x * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions)),
        (float)(tilePos.y * (PhysicsConfig::TerrainSize / TerrainManager::Subdivisions)), 0);
    terrainManager->RenderTile(pos, tilePos - (pos * TerrainManager::Subdivisions), projectionMatrix, mvMatrix);
}

void Region::CleanupRegion(TerrainManager* terrainManager, btDynamicsWorld* dynamicsWorld)
{
	terrainManager->UnloadTerrainTile(pos);
	for (std::pair<const vec::vec2i, btRigidBody*> heightmap : loadedHeightmaps)
	{
		dynamicsWorld->removeRigidBody(heightmap.second);
		delete heightmap.second->getCollisionShape();
		delete heightmap.second->getMotionState();
		delete heightmap.second;
	}
}

Region::~Region()
{
}