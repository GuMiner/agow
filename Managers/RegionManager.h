#pragma once
#include <string>
#include <set>
#include <vector>
#include <GL/glew.h>
#include <Bullet\btBulletDynamicsCommon.h>
#include "shaders\ShaderFactory.h"
#include "Managers\TerrainManager.h"
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include "Physics.h"
#include "Region.h"

class RegionManager
{
    TerrainManager terrainManager;
    std::map<glm::ivec2, Region*, iVec2Comparer> loadedRegions;

    // Each region is subdivided into 100 tiles.
    std::vector<glm::ivec2> visibleTiles;

    // Extents we can actually transverse in the tiles.
    glm::ivec2 min;
    glm::ivec2 max;
    int tileViewDistance;

    glm::ivec2 lastCenterTile;
    void ComputeVisibleTiles(const glm::ivec2& centerTile, const glm::vec2& playerOrientation, std::vector<glm::ivec2>* visibleTiles) const;

public:
    RegionManager(ShaderFactory* shaderManager, ModelManager* modelManager, Physics* physics, std::string terrainRootFolder, glm::ivec2 min, glm::ivec2 max, int tileViewDistance);
    bool InitializeGraphics();
    
    TerrainManager& GetTerrainManager();

    // Returns the sector the user is currently in.
    glm::ivec2 GetCurrentCenterTile(const glm::vec3& position) const;

    // Returns the height of the point, in real units. If the point is out-of-bounds, returns 0 (min height).
    float GetPointHeight(Physics* physics, const glm::vec2 point);

    // Returns the terrain type at the specified point. Returns LAKE if out of bounds.
    int GetPointTerrainType(Physics* physics, const glm::vec2 point);

    void UpdateVisibleRegion(const glm::vec3& playerPosition, const glm::vec2& playerOrientation, Physics* physics);
    void SimulateVisibleRegions(float gameTime, float elapsedSeconds);
    void RenderRegions(const glm::mat4& perspectiveMatrix, const glm::vec3& playerPosition, const glm::vec2& playerDirection, const glm::mat4& viewMatrix);

    void CleanupPhysics(Physics* physics);
    virtual ~RegionManager();
};

