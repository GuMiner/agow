#pragma once
#include <map>
#include <string>
#include <vector>
#include "AI\DecisionTree.h"
#include "Managers\ModelManager.h"
#include "Math\Vec.h"
#include "BasicPhysics.h"

struct BuildingDecisionData
{
    std::string modelName;
    // TODO add in scale information.
};

// Builds buildings by using a decision tree to pseudo-randomly generate realistic structures,
//  scaling appropriately and adding physics.
class BuildingGenerator
{
    static DecisionTree<BuildingDecisionData> buildingBuilder;
    static BuildingDecisionData DeserializeBuildingRule(std::string line, bool* success);
public:
    BuildingGenerator();
    static bool LoadBuilder(std::string builderDecisionTreeFile);

    // TODO
};

