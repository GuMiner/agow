#pragma once
#include <map>
#include <string>
#include <vector>
#include <glm\vec3.hpp>
#include "AI\DecisionTree.h"
#include "Data\Model.h"
#include "Managers\ModelManager.h"
#include "Physics.h"

struct BuildingDecisionData
{
    std::string modelName;
    float zFactor;
    float minScaleFactor;
    float maxScaleFactor;
    int minLayers;
    int maxLayers;
};

// Builds buildings by using a decision tree to pseudo-randomly generate realistic structures,
//  scaling appropriately and adding physics.
class BuildingGenerator
{
    static DecisionTree<BuildingDecisionData> lowDensityBuildingBuilder;
    static DecisionTree<BuildingDecisionData> highDensityBuildingBuilder;

    static BuildingDecisionData DeserializeBuildingRule(std::string line, bool* success);
    static DecisionTree<BuildingDecisionData>::Choice RandomWalkEvaluator(BuildingDecisionData decisionData, bool isValidSegment, bool yesNodeNull, bool noNodeNull);

    ModelManager* modelManager;
    Physics* physics;
    void GetScaledModelPoints(std::vector<glm::vec3>& points, glm::vec3 scaleFactor, unsigned int modelId);

    std::vector<Model> GetRandomBuilding(DecisionTree<BuildingDecisionData>& builder, const glm::vec3& offset, float* buildingFootprintSize, float* height);
public:
    BuildingGenerator(ModelManager* modelManager, Physics* physics);
    static bool LoadBuildingModels(ModelManager* modelManager);
    static bool LoadBuilder(std::string lowDensityFile, std::string highDensityFile);

    // Returns a random low density building centered (XY) on the origin starting at Z == 0.
    std::vector<Model> GetRandomLowDensityBuilding(glm::vec3 offset, float* buildingFootprintSize, float* height);

    // Same as the low-density, but with a high-density building.
    std::vector<Model> GetRandomHighDensityBuilding(glm::vec3 offset, float* buildingFootprintSize, float* height);
};

