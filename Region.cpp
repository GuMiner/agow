#include "Region.h"
#include "Config\PhysicsConfig.h"
#include "Math\MatrixOps.h"

Region::Region(int x, int y, btDynamicsWorld* dynamicsWorld, TerrainManager* terrainManager, int subdivisions, int renderDistance)
    : subdivisions(subdivisions), renderDistance(renderDistance)
{
    terrainManager->LoadTerrainTile(x, y, &tile);
    CreateHeightmap(dynamicsWorld);
}


void Region::CreateHeightmap(btDynamicsWorld* dynamicsWorld)
{
    btHeightfieldTerrainShape* heighfield = new btHeightfieldTerrainShape(
        PhysicsConfig::TerrainSize, PhysicsConfig::TerrainSize, tile->heightmap, 900.0f, 2, true, false);
    heighfield->setMargin(2.0f);

    // Position the heightfield so that it's not repositioned incorrectly.
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(btVector3(
        (tile->x + 0.5f) * PhysicsConfig::TerrainSize,
        (tile->y + 0.5f) * PhysicsConfig::TerrainSize, 450.0f - 2.0f));

    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, heighfield);

    btTransform transform;
    btVector3 min, max;

    heighfield->getAabb(transform, min, max);
    heightmap = new btRigidBody(ground);
    dynamicsWorld->addRigidBody(heightmap);
}

void Region::RenderRegion(TerrainManager* terrainManager, const vec::mat4& projectionMatrix) const
{
    vec::mat4 mvMatrix = MatrixOps::Translate(
        (float)(tile->x * PhysicsConfig::TerrainSize),
        (float)(tile->y * PhysicsConfig::TerrainSize), 0);
    terrainManager->RenderTile(tile->x, tile->y, projectionMatrix, mvMatrix);
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