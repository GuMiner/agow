#include "Region.h"
#include "Config\PhysicsConfig.h"
#include "Math\MatrixOps.h"

Region::Region(vec::vec2i pos, btDynamicsWorld* dynamicsWorld, TerrainManager* terrainManager, int subdivisions)
    : pos(pos), subdivisions(subdivisions)
{
    terrainManager->LoadTerrainTile(pos, &regionTile);
    CreateHeightmap(dynamicsWorld);
}

vec::vec2i Region::GetPos() const
{
	return pos;
}

void Region::CreateHeightmap(btDynamicsWorld* dynamicsWorld)
{
	// TODO -- Load and unload the heightmap based on tiles, not based on regions.
    /*btHeightfieldTerrainShape* heighfield = new btHeightfieldTerrainShape(
        PhysicsConfig::TerrainSize, PhysicsConfig::TerrainSize, regionTile->heightmap, 900.0f, 2, true, false);
    heighfield->setMargin(2.0f);

    // Position the heightfield so that it's not repositioned incorrectly.
    btTransform heightfieldPos;
	heightfieldPos.setIdentity();
	heightfieldPos.setOrigin(btVector3(
        (pos.x + 0.5f) * PhysicsConfig::TerrainSize,
        (pos.y + 0.5f) * PhysicsConfig::TerrainSize, 450.0f - 2.0f));

    btDefaultMotionState *motionState = new btDefaultMotionState(heightfieldPos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, heighfield);

    btTransform transform;
    btVector3 min, max;

    heighfield->getAabb(transform, min, max);
    heightmap = new btRigidBody(ground);
    dynamicsWorld->addRigidBody(heightmap);*/
}

void Region::RenderRegion(vec::vec2i tilePos, TerrainManager* terrainManager, const vec::mat4& projectionMatrix) const
{
    vec::mat4 mvMatrix = MatrixOps::Translate(
        (float)(pos.x * PhysicsConfig::TerrainSize),
        (float)(pos.y * PhysicsConfig::TerrainSize), 0);
    terrainManager->RenderTile(pos, tilePos, projectionMatrix, mvMatrix);
}

void Region::CleanupRegion(TerrainManager* terrainManager, btDynamicsWorld* dynamicsWorld)
{
	terrainManager->UnloadTerrainTile(pos);
    /*dynamicsWorld->removeRigidBody(heightmap);
    delete heightmap->getCollisionShape();
    delete heightmap->getMotionState();
    delete heightmap;*/
}

Region::~Region()
{
}