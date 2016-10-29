#pragma once
#include <Bullet\btBulletDynamicsCommon.h>
#include <Bullet\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h>
#include <string>
#include "Data\TerrainTile.h"
#include "Managers\TerrainManager.h"
#include <glm\vec3.hpp>

// Defines how to display our visible game region.
class Region
{
    glm::ivec2 pos;

    TerrainTile* regionTile;
    std::map<glm::ivec2, btRigidBody*, iVec2Comparer> loadedHeightmaps;

    btRigidBody* CreateHeightmap(glm::ivec2 tilePos, SubTile *subTile, btDynamicsWorld* dynamicsWorld);
     
public:
    Region(glm::ivec2 pos,  TerrainManager* terrainManager);
    glm::ivec2 GetPos() const;

    void EnsureHeightmapsLoaded(btDynamicsWorld* dynamicsWorld, const std::vector<glm::ivec2>* tilesToLoadHeightmapsFor);

    float GetPointHeight(const glm::ivec2 tilePos, const glm::ivec2 fullPos) const;
    int GetPointType(const glm::ivec2 tilePos, const glm::ivec2 fullPos) const;

    void Simulate(TerrainManager* terrainManager, glm::ivec2 tilePos, float elapsedSeconds);
    void RenderRegion(glm::ivec2 tilePos, const glm::vec3& playerPosition, const glm::vec2& playerDirection, TerrainManager* terrainManager, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix) const;

    void CleanupRegion(TerrainManager* terrainManager, btDynamicsWorld* dynamicsWorld);
    virtual ~Region();
};

