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
	vec::vec2i pos;
    int subdivisions;

    TerrainTile* regionTile;
    btRigidBody* heightmap;

    void CreateHeightmap(btDynamicsWorld* dynamicsWorld);

public:
	Region();
    Region(vec::vec2i pos, btDynamicsWorld* dynamicsWorld, TerrainManager* terrainManager, int subdivisions);
	vec::vec2i GetPos() const;

    void RenderRegion(vec::vec2i tilePos, TerrainManager* terrainManager, const vec::mat4& projectionMatrix) const;

    void CleanupRegion(btDynamicsWorld* dynamicsWorld);
    virtual ~Region();
};

