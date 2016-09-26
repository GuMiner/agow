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
    std::map<vec::vec2i, btRigidBody*, vec::vec2iComparer> loadedHeightmaps;

    btRigidBody* CreateHeightmap(vec::vec2i tilePos, SubTile *subTile, btDynamicsWorld* dynamicsWorld);
     
public:
    Region(vec::vec2i pos,  TerrainManager* terrainManager, int subdivisions);
    vec::vec2i GetPos() const;

    void EnsureHeightmapsLoaded(btDynamicsWorld* dynamicsWorld, const std::vector<vec::vec2i>* tilesToLoadHeightmapsFor);

    float GetPointHeight(const vec::vec2i tilePos, const vec::vec2i fullPos) const;
    int GetPointType(const vec::vec2i tilePos, const vec::vec2i fullPos) const;

    void Simulate(TerrainManager* terrainManager, vec::vec2i tilePos, float elapsedSeconds);
    void RenderRegion(vec::vec2i tilePos, TerrainManager* terrainManager, const vec::mat4& perspectiveMatrix, const vec::mat4& viewMatrix) const;

    void CleanupRegion(TerrainManager* terrainManager, btDynamicsWorld* dynamicsWorld);
    virtual ~Region();
};

