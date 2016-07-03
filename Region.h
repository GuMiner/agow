#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include <Bullet\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h>
#include "Math\Vec.h"
#include "Terrain.h"

// Defines how to display our visible game region.
class Region
{
    int subdivisions;
    int renderDistance;

    Terrain terrain;

    TerrainTile* testTile;
    btRigidBody* testBody;

public:
    Region(ShaderManager* shaderManager, std::string terrainRootFolder, int maxTileSize, int tileSize, int subdivisions, int renderDistance);

    // Loads basic regional settings, such as shaders, that are independent of the current position and physics.
    bool LoadBasics();

    void UpdateVisibleRegion(const vec::vec3& playerPosition, btDynamicsWorld* dynamicsWorld);
    void RenderRegion(const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

    void CleanupRegion(btDynamicsWorld* dynamicsWorld);
    virtual ~Region();
};

