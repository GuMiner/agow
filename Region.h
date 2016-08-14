#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include <Bullet\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h>
#include <string>
#include "Data\TerrainTile.h"
#include "Managers\TerrainManager.h"
#include "Math\Vec.h"

// Defines how to display our visible game region.
class Region
{
    // TODO use these actually.
    int subdivisions;
    int renderDistance;

    TerrainTile* tile;
    btRigidBody* heightmap;

    void CreateHeightmap(btDynamicsWorld* dynamicsWorld);

public:
    Region(int x, int y, btDynamicsWorld* dynamicsWorld, TerrainManager* terrainManager, int subdivisions, int renderDistance);
    
    void RenderRegion(TerrainManager* terrainManager, const vec::mat4& projectionMatrix) const;

    void CleanupRegion(btDynamicsWorld* dynamicsWorld);
    virtual ~Region();
};

