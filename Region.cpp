#include "Region.h"
#include "Math\MatrixOps.h"

Region::Region(ShaderManager* shaderManager, std::string terrainRootFolder, int maxTileSize, int tileSize, int subdivisions, int renderDistance)
    : terrain(shaderManager, terrainRootFolder, maxTileSize, tileSize), subdivisions(subdivisions), renderDistance(renderDistance)
{
    testTile = nullptr;
}

// Loads basic regional settings, such as shaders, that are independent of the current position and physics.
bool Region::LoadBasics()
{
    return terrain.LoadBasics();
}

void Region::UpdateVisibleRegion(const vec::vec3& playerPosition, btDynamicsWorld* dynamicsWorld)
{
    // TODO this is definitely going to be dynamic.
    if (testTile == nullptr)
    {
        terrain.LoadTerrainTile(21, 10, &testTile);

        // TODO config file.
        const int heightmapSize = 1000;

        btHeightfieldTerrainShape* heighfield = new btHeightfieldTerrainShape(heightmapSize, heightmapSize, testTile->heightmap, 900.0f, 2, true, false);
        heighfield->setMargin(2.0f);

        // Position the heightfield so that it's not repositioned incorrectly.
        btTransform pos;
        pos.setIdentity();
        pos.setOrigin(btVector3((testTile->x + 0.5f) * terrain.GetTileSize(), (testTile->y + 0.5f) * terrain.GetTileSize(), 450.0f - 2.0f));

        btDefaultMotionState *motionState = new btDefaultMotionState(pos);
        btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, heighfield);

        btTransform transform;
        btVector3 min, max;

        heighfield->getAabb(transform, min, max);
        testBody = new btRigidBody(ground);
        dynamicsWorld->addRigidBody(testBody);
    }
}

void Region::RenderRegion(const vec::mat4& projectionMatrix)
{
    vec::mat4 mvMatrix = MatrixOps::Translate(testTile->x * terrain.GetTileSize(), testTile->y * terrain.GetTileSize(), 0);
    terrain.RenderTile(testTile->x, testTile->y, projectionMatrix, mvMatrix);
}

void Region::CleanupRegion(btDynamicsWorld* dynamicsWorld)
{
    dynamicsWorld->removeRigidBody(testBody);
    delete testBody->getCollisionShape();
    delete testBody->getMotionState();
    delete testBody;
}

Region::~Region()
{
}