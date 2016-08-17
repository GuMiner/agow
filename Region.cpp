#include "Region.h"
#include "Config\PhysicsConfig.h"
#include "Math\MatrixOps.h"

Region::Region()
{}

Region::Region(vec::vec2i pos, btDynamicsWorld* dynamicsWorld, TerrainManager* terrainManager, int subdivisions)
    : pos(pos), subdivisions(subdivisions)
{
    terrainManager->LoadTerrainTile(pos.x, pos.y, &regionTile);
    CreateHeightmap(dynamicsWorld);
}

vec::vec2i Region::GetPos() const
{
	return pos;
}

void Region::CreateHeightmap(btDynamicsWorld* dynamicsWorld)
{
    btHeightfieldTerrainShape* heighfield = new btHeightfieldTerrainShape(
        PhysicsConfig::TerrainSize, PhysicsConfig::TerrainSize, regionTile->heightmap, 900.0f, 2, true, false);
    heighfield->setMargin(2.0f);

    // Position the heightfield so that it's not repositioned incorrectly.
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(btVector3(
        (regionTile->x + 0.5f) * PhysicsConfig::TerrainSize,
        (regionTile->y + 0.5f) * PhysicsConfig::TerrainSize, 450.0f - 2.0f));

    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, heighfield);

    btTransform transform;
    btVector3 min, max;

    heighfield->getAabb(transform, min, max);
    heightmap = new btRigidBody(ground);
    dynamicsWorld->addRigidBody(heightmap);
}

void Region::RenderRegion(vec::vec2i tilePos, TerrainManager* terrainManager, const vec::mat4& projectionMatrix) const
{
    vec::mat4 mvMatrix = MatrixOps::Translate(
        (float)(regionTile->x * PhysicsConfig::TerrainSize),
        (float)(regionTile->y * PhysicsConfig::TerrainSize), 0);
    terrainManager->RenderTile(regionTile->x, regionTile->y, projectionMatrix, mvMatrix);
}

void Region::CleanupRegion(btDynamicsWorld* dynamicsWorld)
{
    dynamicsWorld->removeRigidBody(heightmap);
    delete heightmap->getCollisionShape();
    delete heightmap->getMotionState();
    delete heightmap;
}

Region::~Region()
{
}