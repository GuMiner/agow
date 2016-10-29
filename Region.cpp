#include <glm\gtc\matrix_transform.hpp>
#include "Region.h"
#include "Config\PhysicsConfig.h"
#include "Data\UserPhysics.h"
#include "Utils\TypedCallback.h"

Region::Region(glm::ivec2 pos, TerrainManager* terrainManager)
    : pos(pos)
{
    terrainManager->LoadTerrainTile(pos, &regionTile);
}

glm::ivec2 Region::GetPos() const
{
    return pos;
}

void Region::EnsureHeightmapsLoaded(btDynamicsWorld* dynamicsWorld, const std::vector<glm::ivec2>* tilesToLoadHeightmapsFor)
{
    for (const glm::ivec2& tilePos : *tilesToLoadHeightmapsFor)
    {
        glm::ivec2 localPos = tilePos - (pos * TerrainTile::Subdivisions);
        bool inRegion = (localPos.x >= 0 && localPos.y >= 0 && localPos.x < TerrainTile::Subdivisions && localPos.y < TerrainTile::Subdivisions);
        if (inRegion && loadedHeightmaps.find(localPos) == loadedHeightmaps.end())
        {
            loadedHeightmaps[localPos] = CreateHeightmap(tilePos, regionTile->subtiles[localPos], dynamicsWorld);
        }
    }
}

btRigidBody* Region::CreateHeightmap(glm::ivec2 tilePos, SubTile* subTile, btDynamicsWorld* dynamicsWorld)
{
    btHeightfieldTerrainShape* heightfield = new btHeightfieldTerrainShape(TerrainTile::SubtileSize, TerrainTile::SubtileSize, subTile->heightmap, 900.0f, 2, true, false);
    heightfield->setMargin(2.0f);

    // Position the heightfield so that it's not repositioned incorrectly.
    btTransform heightfieldPos;
    heightfieldPos.setIdentity();
    heightfieldPos.setOrigin(btVector3((float)(tilePos.x + 0.5f) * TerrainTile::SubtileSize, (float)(tilePos.y + 0.5f) * TerrainTile::SubtileSize, 450.0f - 2.0f));

    btDefaultMotionState *motionState = new btDefaultMotionState(heightfieldPos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, heightfield);

    btRigidBody* heightmap =  new btRigidBody(ground);
    heightmap->setFriction(0.50f); // TODO configurable.
    heightmap->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::HEIGHTMAP));

    dynamicsWorld->addRigidBody(heightmap);
    return heightmap;
}

float Region::GetPointHeight(const glm::ivec2 tilePos, const glm::ivec2 fullPos) const
{
    // Map to the correct subtile.
    glm::ivec2 localPos = tilePos - (pos * TerrainTile::Subdivisions);

    // Map to the correct point within the subtile.
    glm::ivec2 subtileOffset = fullPos - (tilePos *  TerrainTile::SubtileSize);
    return regionTile->subtiles[localPos]->heightmap[subtileOffset.x + subtileOffset.y *  TerrainTile::SubtileSize];
}

int Region::GetPointType(const glm::ivec2 tilePos, const glm::ivec2 fullPos) const
{
    // Map to the correct subtile.
    glm::ivec2 localPos = tilePos - (pos * TerrainTile::Subdivisions);

    // Map to the correct point within the subtile.
    glm::ivec2 subtileOffset = fullPos - (tilePos *  TerrainTile::SubtileSize);
    return (int)(regionTile->subtiles[localPos]->type[subtileOffset.x + subtileOffset.y *  TerrainTile::SubtileSize]);
}

void Region::Simulate(TerrainManager* terrainManager, glm::ivec2 tilePos, float elapsedSeconds)
{
    terrainManager->Simulate(pos, tilePos - (pos * TerrainTile::Subdivisions), elapsedSeconds);
}

void Region::RenderRegion(glm::ivec2 tilePos, TerrainManager* terrainManager, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix) const
{
    glm::mat4 mvMatrix = glm::translate(glm::mat4(), glm::vec3((float)(tilePos.x * TerrainTile::SubtileSize), (float)(tilePos.y * TerrainTile::SubtileSize), 0));
    terrainManager->RenderTile(pos, tilePos - (pos * TerrainTile::Subdivisions), perspectiveMatrix, viewMatrix, mvMatrix);
}

void Region::CleanupRegion(TerrainManager* terrainManager, btDynamicsWorld* dynamicsWorld)
{
    terrainManager->UnloadTerrainTile(pos);
    for (std::pair<const glm::ivec2, btRigidBody*> heightmap : loadedHeightmaps)
    {
        dynamicsWorld->removeRigidBody(heightmap.second);
        delete heightmap.second->getUserPointer();
        delete heightmap.second->getCollisionShape();
        delete heightmap.second->getMotionState();
        delete heightmap.second;
    }
}

Region::~Region()
{
}